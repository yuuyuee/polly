// Copyright 2022 The Oak Authors.

#include "oak/common/kafka.h"
#include <string.h>
#include <utility>

#include <librdkafka/rdkafkacpp.h>

#include "oak/logging/logging.h"
#include "oak/common/format.h"

using std::string;
using std::vector;
using std::unique_ptr;

namespace oak {

namespace {
void KafkaEventLogger(const RdKafka::Event& event) {
  switch (event.severity()) {
  case RdKafka::Event::Severity::EVENT_SEVERITY_DEBUG: {
    OAK_DEBUG("Kafka event log: %s: %s\n",
              event.fac().c_str(), event.str().c_str());
    break;
  }

  case RdKafka::Event::Severity::EVENT_SEVERITY_INFO:
  case RdKafka::Event::Severity::EVENT_SEVERITY_NOTICE: {
    OAK_INFO("Kafka event log: %s: %s\n",
              event.fac().c_str(), event.str().c_str());
    break;
  }

  case RdKafka::Event::Severity::EVENT_SEVERITY_WARNING: {
    OAK_WARNING("Kafka event log: %s: %s\n",
                event.fac().c_str(), event.str().c_str());
    break;
  }

  case RdKafka::Event::Severity::EVENT_SEVERITY_ERROR: {
    OAK_ERROR("Kafka event log: %s: %s\n",
              event.fac().c_str(), event.str().c_str());
    break;
  }

  case RdKafka::Event::Severity::EVENT_SEVERITY_CRITICAL:
  case RdKafka::Event::Severity::EVENT_SEVERITY_ALERT:
  case RdKafka::Event::Severity::EVENT_SEVERITY_EMERG: {
    OAK_FATAL("Kafka event log: %s: %s\n",
              event.fac().c_str(), event.str().c_str());
    break;
  }

  default:
    assert(false && "unreachable code");
  }
}

class KafkaEventCallback: public RdKafka::EventCb {
 public:
  virtual ~KafkaEventCallback() {}
  virtual void event_cb(RdKafka::Event& event);
};

void KafkaEventCallback::event_cb(RdKafka::Event& event) {
  switch (event.type()) {
  case RdKafka::Event::EVENT_ERROR: {
    OAK_FATAL("Kafka event error (%s): %s\n",
              RdKafka::err2str(event.err()).c_str(),
              event.str().c_str());
    break;
  }

  case RdKafka::Event::EVENT_STATS: {
    OAK_INFO("Kafka event stats: %s\n", event.str().c_str());
    break;
  }

  case RdKafka::Event::EVENT_LOG: {
    KafkaEventLogger(event);
    break;
  }

  case RdKafka::Event::EVENT_THROTTLE: {
    OAK_INFO("Kafka event throttle: %d ms by %s (%%%d)\n",
             event.throttle_time(),
             event.broker_name().c_str(),
             event.broker_id());
    break;
  }

  default: {
    OAK_INFO("Kafka event %d (%s): %s\n",
             event.type(),
             RdKafka::err2str(event.err()).c_str(),
             event.str().c_str());
    break;
  }
  }
}

class KafkaDeliveryReportCallback: public RdKafka::DeliveryReportCb {
 public:
  virtual ~KafkaDeliveryReportCallback() {}

  virtual void dr_cb(RdKafka::Message& message);
};

void KafkaDeliveryReportCallback::dr_cb(RdKafka::Message& message) {
  if (message.err()) {
    OAK_ERROR("Kafka message delivery failed: %s\n",
              message.errstr().c_str());
  }
}

void SetProperty(RdKafka::Conf* conf, const struct oak_dict& config) {
  string errstr;

  const char* prefix = "kafka.";
  for (size_t i = 0; i < config.size; ++i) {
    StringPiece key(static_cast<const char*>(config.elems[i].key.ptr),
                    config.elems[i].key.size);
    StringPiece value(static_cast<const char*>(config.elems[i].value.ptr),
                      config.elems[i].value.size);
    if (key.starts_with(prefix)) {
      key.remove_prefix(strlen(prefix));
      if (!key.empty() && !value.empty()) {
        if (conf->set(key.data(), value.data(), errstr) !=
            RdKafka::Conf::CONF_OK) {
          OAK_ERROR("Kafka set property (%s: %s): %s\n",
                    key.data(), value.data(), errstr.c_str());
        }
      }
    }
  }
}

}  // anonymous namespace

// KafkaConsumer

class KafkaConsumer::Impl {
 public:
  explicit Impl(const struct oak_dict& config);
  ~Impl();

  void Subscribe(const std::vector<std::string>& topics);
  void Consume(MessageHandler&& handler, int timeout_ms);

 private:
  Impl(Impl const&) = delete;
  Impl& operator=(Impl const&) = delete;

  KafkaEventCallback event_cb_;
  unique_ptr<RdKafka::KafkaConsumer> consumer_;
};

KafkaConsumer::Impl::Impl(const struct oak_dict& config)
    : event_cb_(), consumer_() {
  unique_ptr<RdKafka::Conf> conf;
  conf.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  SetProperty(conf.get(), config);
  string errstr;

  if (conf->set("event_cb", &event_cb_, errstr) != RdKafka::Conf::CONF_OK)
    OAK_ERROR("Kafka set property (event_cb): %s\n", errstr.c_str());

  RdKafka::KafkaConsumer* consumer =
      RdKafka::KafkaConsumer::create(conf.get(), errstr);
  if (!consumer)
    OAK_FATAL("Kafka create consumer failed: %s\n", errstr.c_str());

  consumer_.reset(consumer);
}

KafkaConsumer::Impl::~Impl() {
  consumer_->close();
  consumer_.reset();
  RdKafka::wait_destroyed(500);
}

void KafkaConsumer::Impl::Subscribe(const std::vector<std::string>& topics) {
  RdKafka::ErrorCode err = consumer_->subscribe(topics);
  if (err) {
    OAK_FATAL("Kafk subscribe topics failed: %s\n",
              RdKafka::err2str(err).c_str());
  }
}

void KafkaConsumer::Impl::Consume(MessageHandler&& handler, int timeout_ms) {
  RdKafka::Message *ptr = consumer_->consume(timeout_ms);
  if (!ptr)
    return;

  unique_ptr<RdKafka::Message> message(ptr);
  switch (message->err()) {
  case RdKafka::ERR__TIMED_OUT:
  case RdKafka::ERR__PARTITION_EOF: {   // last message
    break;
  }

  case RdKafka::ERR_NO_ERROR: {
    if (handler) {
      StringPiece key(*(message->key()));
      StringPiece value(static_cast<const char*>(message->payload()),
                        message->len());
      handler(key, value);
    }
    break;
  }

  default: {
    OAK_ERROR("Kafka consume failed: %s\n", message->errstr().c_str());
    break;
  }
  }
}

KafkaConsumer::KafkaConsumer(const struct oak_dict& config)
    : impl_(new Impl(config)) {}

KafkaConsumer::~KafkaConsumer() {}

void KafkaConsumer::Subscribe(const std::vector<std::string>& topics) {
  impl_->Subscribe(topics);
}

void KafkaConsumer::Consume(MessageHandler&& handler, int timeout_ms) {
  impl_->Consume(std::move(handler), timeout_ms);
}

// KafkaProducer

class KafkaProducer::Impl {
 public:
  explicit Impl(const struct oak_dict& config);
  ~Impl();

  void Pruduce(const string& topic,
               const string& key,
               const string& value);

 private:
  Impl(Impl const&) = delete;
  Impl& operator=(Impl const&) = delete;

  KafkaEventCallback event_cb_;
  KafkaDeliveryReportCallback delivery_cb_;
  unique_ptr<RdKafka::Producer> producer_;
};

KafkaProducer::Impl::Impl(const struct oak_dict& config)
    : event_cb_(), delivery_cb_(), producer_() {
  unique_ptr<RdKafka::Conf> conf;
  conf.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  SetProperty(conf.get(), config);
  string errstr;
  if (conf->set("event_cb", &event_cb_, errstr) != RdKafka::Conf::CONF_OK)
    OAK_ERROR("Kafka set property (event_cb): %s\n", errstr.c_str());

  if (conf->set("dr_cb", &delivery_cb_, errstr) != RdKafka::Conf::CONF_OK)
    OAK_ERROR("Kafka set property (dr_cb: %s\n", errstr.c_str());

  RdKafka::Producer* producer =
      RdKafka::Producer::create(conf.get(), errstr);
  if (!producer)
    OAK_FATAL("Kafka create producer failed: %s\n", errstr.c_str());
  producer_.reset(producer);
}

KafkaProducer::Impl::~Impl() {
  producer_->flush(500);
  if (producer_->outq_len() > 0)
    OAK_WARNING("Kafka %d messages not delivered\n", producer_->outq_len());
  producer_.reset();
  RdKafka::wait_destroyed(500);
}

void KafkaProducer::Impl::Pruduce(const string& topic,
                                  const string& key,
                                  const string& value) {
retry:
  RdKafka::ErrorCode err =
      producer_->produce(topic,
                         RdKafka::Topic::PARTITION_UA,
                         RdKafka::Producer::RK_MSG_COPY,
                         const_cast<char*>(value.c_str()),
                         value.size(),
                         const_cast<char*>(key.c_str()),
                         key.size(),
                         0,
                         nullptr);
  if (err != RdKafka::ERR_NO_ERROR) {
    OAK_ERROR("Kafka produce to topic %s failed: %s\n",
              topic.c_str(), RdKafka::err2str(err).c_str());
    if (err == RdKafka::ERR__QUEUE_FULL) {
      producer_->poll(1000);
      goto retry;
    }
  }
  producer_->poll(0);
}

KafkaProducer::KafkaProducer(const struct oak_dict& config)
    : impl_(new Impl(config)) {}

KafkaProducer::~KafkaProducer() {}

void KafkaProducer::Pruduce(const string& topic,
                            const string& key,
                            const string& value) {
  impl_->Pruduce(topic, key, value);
}

}  // namespace oak
