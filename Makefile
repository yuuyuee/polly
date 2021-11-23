string_view_test_SRC = polly/stubs/string_view_test.o \
					   polly/stubs/check.o \
					   polly/stubs/exception.o \
					   third_party/gtest/gmock-gtest-all.o

string_view_test: $(string_view_test_SRC)
	g++ -std=c++11 -g -Wall -Wextra -o $@ $^ -lpthread

%.o: %.cc
	g++ -std=c++11 -g -Wall -Wextra -c -o $@ $^ -Ipolly -Ithird_party/gtest

clean:
	rm -rf *.o string_view_test
	@(find . -name "*.o" | xargs -r rm)

.PHONY: string_view_test
