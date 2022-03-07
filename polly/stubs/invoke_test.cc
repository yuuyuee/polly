// Copyright (c) 2022 The Polly Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "polly/stubs/internal/config.h"

#include <memory>
#include <string>

#include "polly/stubs/invoke.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

int Function(int a, int b) { return a - b; }

int Sink(std::unique_ptr<int> p) {
  return *p;
}

std::unique_ptr<int> Factory(int n) {
  return std::unique_ptr<int>(new int(n));
}

void NoOp() {}

struct ConstFunctor {
  int operator()(int a, int b) const { return a - b; }
};

struct MutableFunctor {
  int operator()(int a, int b) { return a - b; }
};

struct EphemeralFunctor {
  int operator()(int a, int b) && { return a - b; }
};

struct OverloadedFunctor {
  template <typename... Args>
  std::string operator()(const Args&...) & {
    return "&";
  }
  template <typename... Args>
  std::string operator()(const Args&...) const& {
    return "const&";
  }
  template <typename... Args>
  std::string operator()(const Args&...) && {
    return "&&";
  }
};

struct Class {
  int Method(int a, int b) { return a - b; }
  int ConstMethod(int a, int b) const { return a - b; }
  int RefMethod(int a, int b) & { return a - b; }
  int RefRefMethod(int a, int b) && { return a - b; }
  int NoExceptMethod(int a, int b) noexcept { return a - b; }
  int VolatileMethod(int a, int b) volatile { return a - b; }

  int member;
};

struct FlipFlop {
  int ConstMethod() const { return member; }
  FlipFlop operator*() const { return {-member}; }

  int member;
};

// CallMaybeWithArg(f) resolves either to invoke(f) or invoke(f, 42), depending
// on which one is valid.
template <typename F>
decltype(polly::invoke(std::declval<const F&>())) CallMaybeWithArg(
    const F& f) {
  return polly::invoke(f);
}

template <typename F>
decltype(polly::invoke(std::declval<const F&>(), 42)) CallMaybeWithArg(
    const F& f) {
  return polly::invoke(f, 42);
}

TEST(InvokeTest, Function) {
  EXPECT_EQ(1, polly::invoke(Function, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Function, 3, 2));
}

TEST(InvokeTest, NonCopyableArgument) {
  EXPECT_EQ(42, polly::invoke(Sink, std::unique_ptr<int>(new int(42))));
}

TEST(InvokeTest, NonCopyableResult) {
  EXPECT_THAT(polly::invoke(Factory, 42), ::testing::Pointee(42));
}

TEST(InvokeTest, VoidResult) { polly::invoke(NoOp); }

TEST(InvokeTest, ConstFunctor) {
  EXPECT_EQ(1, polly::invoke(ConstFunctor(), 3, 2));
}

TEST(InvokeTest, MutableFunctor) {
  MutableFunctor f;
  EXPECT_EQ(1, polly::invoke(f, 3, 2));
  EXPECT_EQ(1, polly::invoke(MutableFunctor(), 3, 2));
}

TEST(InvokeTest, EphemeralFunctor) {
  EphemeralFunctor f;
  EXPECT_EQ(1, polly::invoke(std::move(f), 3, 2));
  EXPECT_EQ(1, polly::invoke(EphemeralFunctor(), 3, 2));
}

TEST(InvokeTest, OverloadedFunctor) {
  OverloadedFunctor f;
  const OverloadedFunctor& cf = f;

  EXPECT_EQ("&", polly::invoke(f));
  EXPECT_EQ("&", polly::invoke(f, " 42"));

  EXPECT_EQ("const&", polly::invoke(cf));
  EXPECT_EQ("const&", polly::invoke(cf, " 42"));

  EXPECT_EQ("&&", polly::invoke(std::move(f)));

  OverloadedFunctor f2;
  EXPECT_EQ("&&", polly::invoke(std::move(f2), " 42"));
}

TEST(InvokeTest, ReferenceWrapper) {
  ConstFunctor cf;
  MutableFunctor mf;
  EXPECT_EQ(1, polly::invoke(std::cref(cf), 3, 2));
  EXPECT_EQ(1, polly::invoke(std::ref(cf), 3, 2));
  EXPECT_EQ(1, polly::invoke(std::ref(mf), 3, 2));
}

TEST(InvokeTest, MemberFunction) {
  std::unique_ptr<Class> p(new Class);
  std::unique_ptr<const Class> cp(new Class);
  std::unique_ptr<volatile Class> vp(new Class);

  EXPECT_EQ(1, polly::invoke(&Class::Method, p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::Method, p.get(), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::Method, *p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::RefMethod, p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::RefMethod, p.get(), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::RefMethod, *p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::RefRefMethod, std::move(*p), 3,
                                     2));  // NOLINT
  EXPECT_EQ(1, polly::invoke(&Class::NoExceptMethod, p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::NoExceptMethod, p.get(), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::NoExceptMethod, *p, 3, 2));

  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, p.get(), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, *p, 3, 2));

  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, cp, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, cp.get(), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, *cp, 3, 2));

  EXPECT_EQ(1, polly::invoke(&Class::VolatileMethod, p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::VolatileMethod, p.get(), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::VolatileMethod, *p, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::VolatileMethod, vp, 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::VolatileMethod, vp.get(), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::VolatileMethod, *vp, 3, 2));

  EXPECT_EQ(1, polly::invoke(&Class::Method, std::unique_ptr<Class>(new Class), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, std::unique_ptr<Class>(new Class), 3, 2));
  EXPECT_EQ(1, polly::invoke(&Class::ConstMethod, std::unique_ptr<const Class>(new Class), 3, 2));
}

TEST(InvokeTest, DataMember) {
  std::unique_ptr<Class> p(new Class{42});
  std::unique_ptr<const Class> cp(new Class{42});
  EXPECT_EQ(42, polly::invoke(&Class::member, p));
  EXPECT_EQ(42, polly::invoke(&Class::member, *p));
  EXPECT_EQ(42, polly::invoke(&Class::member, p.get()));

  polly::invoke(&Class::member, p) = 42;
  polly::invoke(&Class::member, p.get()) = 42;

  EXPECT_EQ(42, polly::invoke(&Class::member, cp));
  EXPECT_EQ(42, polly::invoke(&Class::member, *cp));
  EXPECT_EQ(42, polly::invoke(&Class::member, cp.get()));
}

TEST(InvokeTest, FlipFlop) {
  FlipFlop obj = {42};
  // This call could resolve to (obj.*&FlipFlop::ConstMethod)() or
  // ((*obj).*&FlipFlop::ConstMethod)(). We verify that it's the former.
  EXPECT_EQ(42, polly::invoke(&FlipFlop::ConstMethod, obj));
  EXPECT_EQ(42, polly::invoke(&FlipFlop::member, obj));
}

TEST(InvokeTest, SfinaeFriendly) {
  CallMaybeWithArg(NoOp);
  EXPECT_THAT(CallMaybeWithArg(Factory), ::testing::Pointee(42));
}

}  // anonymous namespace
