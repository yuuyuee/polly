TARGETS = polly_stubs_test string_view_test hash_test type_traits_test optional_test any_test invoke_test variant_test span_test
DEFAULT_TARGETS = polly_stubs_test

string_view_test_SRC = polly/stubs/string_view_test.cc \
		polly/stubs/internal/raw_logging.cc            \
		polly/stubs/internal/throw_delegate.cc         \
		polly/stubs/hash.cc                            \
		third_party/gtest/gtest_main.cc                \
		third_party/gtest/gmock-gtest-all.cc

hash_test_SRC = polly/stubs/hash_test.cc         \
		polly/stubs/hash.cc                      \
		third_party/gtest/gtest_main.cc          \
		third_party/gtest/gmock-gtest-all.cc

optional_test_SRC = polly/stubs/optional_test.cc \
		polly/stubs/internal/raw_logging.cc      \
		polly/stubs/internal/throw_delegate.cc   \
		third_party/gtest/gtest_main.cc          \
		third_party/gtest/gmock-gtest-all.cc

any_test_SRC = polly/stubs/any_test.cc           \
		polly/stubs/internal/raw_logging.cc      \
		polly/stubs/internal/throw_delegate.cc   \
		third_party/gtest/gtest_main.cc          \
		third_party/gtest/gmock-gtest-all.cc

type_traits_test_SRC = polly/stubs/type_traits_test.cc \
		third_party/gtest/gtest_main.cc                \
		third_party/gtest/gmock-gtest-all.cc

invoke_test_SRC = polly/stubs/invoke_test.cc	 \
		third_party/gtest/gtest_main.cc          \
		third_party/gtest/gmock-gtest-all.cc

variant_test_SRC = polly/stubs/variant_test.cc   \
		polly/stubs/internal/raw_logging.cc      \
		polly/stubs/internal/throw_delegate.cc   \
		third_party/gtest/gtest_main.cc          \
		third_party/gtest/gmock-gtest-all.cc

span_test_SRC = polly/stubs/span_test.cc         \
		polly/stubs/internal/raw_logging.cc      \
		polly/stubs/internal/throw_delegate.cc   \
		third_party/gtest/gtest_main.cc          \
		third_party/gtest/gmock-gtest-all.cc

polly_stubs_test_SRC =                          \
		polly/stubs/internal/raw_logging.cc     \
		polly/stubs/internal/throw_delegate.cc  \
		polly/stubs/hash.cc                     \
		polly/stubs/type_traits_test.cc         \
		polly/stubs/string_view_test.cc         \
		polly/stubs/optional_test.cc            \
		polly/stubs/any_test.cc                 \
		polly/stubs/hash_test.cc                \
		polly/stubs/invoke_test.cc              \
		polly/stubs/variant_test.cc             \
		polly/stubs/span_test.cc                \
		third_party/gtest/gtest_main.cc         \
		third_party/gtest/gmock-gtest-all.cc


CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -g -I. -Ithird_party/gtest #-fno-exceptions #-fno-rtti
LDFLAGS ?=
LDADD ?= -rdynamic -lpthread

define BUILD_TARGET
$1: $$(patsubst %.cc, %.o, $$($1_SRC))
	g++ $$(LDFLAGS) -o $$@ $$^ $$(LDADD)

$1-CLEAN:
	@(echo "rm $1"; if test -e $1; then rm $1; fi;)

.PHONY: $1 $1-CLEAN
endef

all: $(DEFAULT_TARGETS);

clean: $(foreach t, $(TARGETS), $t-CLEAN)
	@(echo "rm *.o"; find . -name "*.o" | xargs -r rm)

%.o: %.cc
	g++ $(CXXFLAGS) -c -o $@ $^

$(foreach t, $(TARGETS), $(eval $(call BUILD_TARGET,$t)))

