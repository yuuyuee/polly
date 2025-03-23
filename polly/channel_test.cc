// Copyright 2022 The Oak Authors.

#include <oak/common/channel.h>

#include <chrono>   // NOLINT
#include <thread>   // NOLINT
#include <string>

#include "gtest/gtest.h"

namespace {

TEST(QueueTest, ClassSize) {
  EXPECT_EQ(sizeof(oak::Channel) % OAK_CACHELINE_SIZE, 0U);
  oak::Channel* ptr = new oak::Channel(1 << 2);
  uint64_t base = reinterpret_cast<uint64_t>(ptr);
  EXPECT_EQ(base & (OAK_CACHELINE_SIZE - 1), 0U);
  delete ptr;
}

template <typename Tp>
struct TestTraits {
  std::atomic<size_t> generate_count{0};
  std::atomic<size_t> free_count{0};

  size_t Limits() { return 1 << 20; }

  template <typename Up>
  Tp* GenerateHelper(const Up*) {
    generate_count.fetch_add(1, std::memory_order_acq_rel);
    return new Tp(rand() % 25);
  }

  Tp* GenerateHelper(const std::string*) {
    generate_count.fetch_add(1, std::memory_order_acq_rel);
    return new Tp("    ");
  }

  Tp* Generate() {
    return GenerateHelper(static_cast<Tp*>(0));
  }

  void Free(Tp* ptr) {
    free_count.fetch_add(1, std::memory_order_acq_rel);
    delete ptr;
  }
};

template <typename Tp, size_t Size>
struct PerfTest {
  PerfTest() : channel_(Size), done_(false) {}

  void operator()() {
    auto const start_time = std::chrono::system_clock::now();

    std::thread producer0([this] { prod_count_0 = this->Producer(); });
    std::thread producer1([this] { prod_count_1 = this->Producer(); });
    std::thread consumer0([this] { cons_count_0 = this->Consumer(); });
    std::thread consumer1([this] { cons_count_1 = this->Consumer(); });

    producer0.join();
    producer1.join();
    done_ = true;
    consumer0.join();
    consumer1.join();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - start_time);
  }

  uint64_t Producer() {
    uint64_t count = 0;
    for (size_t i = 0; i < traits.Limits(); ++i) {
      Tp* ptr = traits.Generate();
      assert(ptr != 0);
      while (!channel_.Push(ptr)) {
        // continue;
      }
      ++count;
    }
    return count;
  }

  uint64_t Consumer() {
    Tp* ptr;
    uint64_t count = 0;
    while (!done_) {
      if (channel_.Pop(&ptr)) {
        ++count;
        traits.Free(ptr);
        assert(ptr != 0);
      }
    }

    while (channel_.Pop(&ptr)) {
      ++count;
      traits.Free(ptr);
      assert(ptr != 0);
    }
    return count;
  }

  oak::Channel channel_;
  TestTraits<Tp> traits;
  std::atomic<bool> done_;
  uint64_t prod_count_0{0};
  uint64_t prod_count_1{0};
  uint64_t cons_count_0{0};
  uint64_t cons_count_1{0};
};

TEST(QueueTest, PerfTest) {
  {
    PerfTest<int, 1 << 16> test;
    test();
    EXPECT_EQ(test.prod_count_0 + test.prod_count_1,
              test.cons_count_0 + test.cons_count_1);
    EXPECT_EQ(test.traits.generate_count, test.traits.free_count);
  }

  {
    PerfTest<std::string, 1 << 16> test;
    test();
    EXPECT_EQ(test.prod_count_0 + test.prod_count_1,
              test.cons_count_0 + test.cons_count_1);
    EXPECT_EQ(test.traits.generate_count, test.traits.free_count);
  }

  {
    PerfTest<uint64_t, 1 << 16> test;
    test();
    EXPECT_EQ(test.prod_count_0 + test.prod_count_1,
              test.cons_count_0 + test.cons_count_1);
    EXPECT_EQ(test.traits.generate_count, test.traits.free_count);
  }
}

TEST(QueueTest, UsedSpaceAvailSpace) {
  oak::Channel channel(4);
  void* ptr = reinterpret_cast<void*>(0);

  // UsedSpace round 1
  EXPECT_EQ(channel.UsedSpace(), 0U);

  EXPECT_EQ(channel.Push(ptr), true);
  EXPECT_EQ(channel.UsedSpace(), 1U);

  EXPECT_EQ(channel.Push(ptr), true);
  EXPECT_EQ(channel.UsedSpace(), 2U);

  EXPECT_EQ(channel.Push(ptr), true);
  EXPECT_EQ(channel.UsedSpace(), 3U);

  EXPECT_EQ(channel.Push(ptr), false);
  EXPECT_EQ(channel.UsedSpace(), 3U);

  // AvailSpace round 1
  EXPECT_EQ(channel.AvailSpace(), 0U);

  EXPECT_EQ(channel.Pop(&ptr), true);
  EXPECT_EQ(channel.AvailSpace(), 1U);

  EXPECT_EQ(channel.Pop(&ptr), true);
  EXPECT_EQ(channel.AvailSpace(), 2U);

  EXPECT_EQ(channel.Pop(&ptr), true);
  EXPECT_EQ(channel.AvailSpace(), 3U);

  EXPECT_EQ(channel.Pop(&ptr), false);
  EXPECT_EQ(channel.AvailSpace(), 3U);

  // UsedSpace round 2
  EXPECT_EQ(channel.UsedSpace(), 0U);

  EXPECT_EQ(channel.Push(ptr), true);
  EXPECT_EQ(channel.UsedSpace(), 1U);

  EXPECT_EQ(channel.Push(ptr), true);
  EXPECT_EQ(channel.UsedSpace(), 2U);

  EXPECT_EQ(channel.Push(ptr), true);
  EXPECT_EQ(channel.UsedSpace(), 3U);

  EXPECT_EQ(channel.Push(ptr), false);
  EXPECT_EQ(channel.UsedSpace(), 3U);

  // AvailSpace round 2
  EXPECT_EQ(channel.AvailSpace(), 0U);

  EXPECT_EQ(channel.Pop(&ptr), true);
  EXPECT_EQ(channel.AvailSpace(), 1U);

  EXPECT_EQ(channel.Pop(&ptr), true);
  EXPECT_EQ(channel.AvailSpace(), 2U);

  EXPECT_EQ(channel.Pop(&ptr), true);
  EXPECT_EQ(channel.AvailSpace(), 3U);

  EXPECT_EQ(channel.Pop(&ptr), false);
  EXPECT_EQ(channel.AvailSpace(), 3U);
}


TEST(QueueTest, Capacity) {
  oak::Channel channel(4);
  EXPECT_EQ(channel.Capacity(), 3U);
}

}  // anonymous namespace
