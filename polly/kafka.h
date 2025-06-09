// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#ifndef POLLY_COMMON_KAFKA_H_
#define POLLY_COMMON_KAFKA_H_

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "polly/stringpiece.h"
#include "polly/logging.h"

namespace polly {

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

}  // namespace polly

#endif  // POLLY_COMMON_KAFKA_H_
