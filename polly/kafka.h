// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_KAFKA_H_
#define OAK_COMMON_KAFKA_H_

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "oak/addons/dict_internal.h"
#include "oak/common/stringpiece.h"
#include "oak/logging/logging.h"

namespace oak {

using MessageHandler =
    std::function<void(const StringPiece, const StringPiece)>;

class KafkaConsumer {
 public:
  explicit KafkaConsumer(const struct oak_dict& config);
  ~KafkaConsumer();

  void Subscribe(const std::vector<std::string>& topics);

  void Consume(MessageHandler&& handler, int timeout_ms);

 private:
  KafkaConsumer(KafkaConsumer const&) = delete;
  KafkaConsumer& operator=(KafkaConsumer const&) = delete;

  class Impl;
  std::unique_ptr<Impl> impl_;
};

class KafkaProducer {
 public:
  explicit KafkaProducer(const struct oak_dict& config);
  ~KafkaProducer();

  void Pruduce(const std::string& topic,
               const std::string& key,
               const std::string& value);

 private:
  KafkaProducer(KafkaProducer const&) = delete;
  KafkaProducer& operator=(KafkaProducer const&) = delete;

  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace oak

#endif  // OAK_COMMON_KAFKA_H_
