// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_CHANNEL_H_
#define OAK_COMMON_CHANNEL_H_

#include <assert.h>
#include <stdint.h>
#include <atomic>

#include "oak/common/macros.h"

namespace oak {

// Queue
// This class is a multi producer and multi consumer queue without locks.

class alignas(OAK_CACHELINE_SIZE) Channel {
 public:
  // Construct object with a fixed maximum size.
  // NOTE: the size MUST be a pwer of 2.
  explicit Channel(uint32_t size, void** ptr = nullptr) noexcept
      : ownership_(false), size_(size), records_(ptr),
        cons_head_(0), cons_tail_(0),
        prod_head_(0), prod_tail_(0) {
    assert(((size & (size - 1)) == 0) && "size MUST be a pwer of 2");
    if (!records_) {
      records_ = ::new void*[size];
      assert(records_ && "No enough memory");
      ownership_ = true;
    }
  }

  // Destruct object.
  ~Channel() {
    if (ownership_)
      ::delete[](records_);
  }

  inline bool Push(void* ptr) {
    auto prod_head = prod_head_.load(std::memory_order_relaxed);
    auto next_prod_head = 0U;

    do {
      next_prod_head = prod_head + 1;
      if (next_prod_head == size_)
        next_prod_head = 0;

      if (next_prod_head == cons_tail_.load(std::memory_order_acquire))
        return false;
    } while (!prod_head_.compare_exchange_weak(
             prod_head, next_prod_head,
             std::memory_order_acq_rel,
             std::memory_order_relaxed));

    records_[prod_head] = ptr;
    std::atomic_thread_fence(std::memory_order_release);

    while (prod_tail_.load(std::memory_order_relaxed) != prod_head) {
      // wait for completion of the other thread
      assert(prod_tail_ < size_);
    }
    prod_tail_.store(next_prod_head, std::memory_order_release);
    return true;
  }

  template <typename Tp>
  inline bool Pop(Tp** ptr) {
    auto cons_head = cons_head_.load(std::memory_order_relaxed);
    auto next_cons_head = 0U;

    do {
      if (cons_head == prod_tail_.load(std::memory_order_acquire))
        return false;

      next_cons_head = cons_head + 1;
      if (next_cons_head == size_)
        next_cons_head = 0;
    } while (!cons_head_.compare_exchange_weak(
             cons_head, next_cons_head,
             std::memory_order_acq_rel,
             std::memory_order_relaxed));

    std::atomic_thread_fence(std::memory_order_acquire);
    *ptr = static_cast<Tp*>(records_[cons_head]);

    while (cons_tail_.load(std::memory_order_relaxed) != cons_head) {
      // wait for completion of the other thread
      assert(cons_tail_ < size_);
    }
    cons_tail_.store(next_cons_head, std::memory_order_release);
    return true;
  }

  // Return the number of used space in the channel.
  constexpr size_t UsedSpace() const {
    return (prod_tail_.load(std::memory_order_acquire) -
           cons_head_.load(std::memory_order_acquire)) &
           (size_ - 1);
  }

  // Return the number of available space in the channel.
  constexpr size_t AvailSpace() const {
    return size_ - 1 - UsedSpace();
  }

  // Maximum number of items in the queue.
  constexpr size_t Capacity() const { return size_ - 1; }

 private:
  Channel(Channel const&) = delete;
  Channel& operator=(const Channel&) = delete;

  char padding0_[OAK_CACHELINE_SIZE];

  bool ownership_;
  const uint32_t size_;
  void** records_;

  char padding1_[OAK_CACHELINE_SIZE -
                 sizeof(ownership_) -
                 sizeof(size_) -
                 sizeof(records_)];

  alignas(OAK_CACHELINE_SIZE) std::atomic<uint32_t> cons_head_;
  char padding2_[OAK_CACHELINE_SIZE - sizeof(cons_head_)];

  alignas(OAK_CACHELINE_SIZE) std::atomic<uint32_t> cons_tail_;
  char padding3_[OAK_CACHELINE_SIZE - sizeof(cons_tail_)];

  alignas(OAK_CACHELINE_SIZE) std::atomic<uint32_t> prod_head_;
  char padding4_[OAK_CACHELINE_SIZE - sizeof(prod_head_)];

  alignas(OAK_CACHELINE_SIZE) std::atomic<uint32_t> prod_tail_;
  char padding5_[OAK_CACHELINE_SIZE - sizeof(prod_tail_)];
};

}  // namespace oak

#endif  // OAK_COMMON_CHANNEL_H_

