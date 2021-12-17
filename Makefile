TARGETS = polly_stubs_test string_view_test hash_test type_traits_test optional_test

string_view_test_SRC = polly/stubs/string_view_test.o \
		polly/stubs/internal/raw_logging.o            \
		polly/stubs/internal/throw_delegate.o         \
		polly/stubs/hash.o                            \
		third_party/gtest/gtest_main.o                \
		third_party/gtest/gmock-gtest-all.o

hash_test_SRC = polly/stubs/hash_test.o \
		polly/stubs/hash.o              \
		third_party/gtest/gtest_main.o  \
		third_party/gtest/gmock-gtest-all.o

optional_test_SRC = polly/stubs/optional_test.o \
		polly/stubs/internal/raw_logging.o      \
		polly/stubs/internal/throw_delegate.o   \
		third_party/gtest/gtest_main.o          \
		third_party/gtest/gmock-gtest-all.o

type_traits_test_SRC = polly/stubs/type_traits_test.o \
		third_party/gtest/gtest_main.o                \
		third_party/gtest/gmock-gtest-all.o

polly_stubs_test_SRC =                          \
		polly/stubs/string_view_test.cc         \
		polly/stubs/optional_test.cc            \
		polly/stubs/hash_test.cc                \
		polly/stubs/type_traits_test.cc         \
		polly/stubs/internal/raw_logging.cc     \
		polly/stubs/internal/throw_delegate.cc  \
		polly/stubs/hash.cc                     \
		third_party/gtest/gtest_main.cc         \
		third_party/gtest/gmock-gtest-all.cc


define BUILD_TARGET
$1: $$(patsubst %.cc, %.o, $$($1_SRC))
	g++ -std=c++11 -g -Wall -Wextra -o $$@ $$^ -lpthread

$1-CLEAN:
	@(echo "rm $1"; if test -e $1; then rm $1; fi;)

.PHONY: $1 $1-CLEAN
endef

all: $(TARGETS);

clean: $(foreach t, $(TARGETS), $t-CLEAN)
	@(echo "rm *.o"; find . -name "*.o" | xargs -r rm)

%.o: %.cc
	g++ -std=c++11 -g -Wall -Wextra -c -o $@ $^ -Ipolly -Ithird_party/gtest

$(foreach t, $(TARGETS), $(eval $(call BUILD_TARGET,$t)))

