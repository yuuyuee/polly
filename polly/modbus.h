// Copyright 2022 The Oak Authors.

#ifndef POLLY_MODBUS_H_
#define POLLY_MODBUS_H_

struct io_context;

// Wrapped the real implementation
// Abstracted the behaviors
class Modbus {
 public:
    explicit Modbus(io_context* io_ctx);

 private:
};

#endif  // POLLY_MODBUS_H_
