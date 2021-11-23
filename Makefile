string_view_test_SRC = polly/stubs/string_view_test.o 	\
					   polly/stubs/check.o 				\
					   polly/stubs/hash.o 				\
					   polly/stubs/exception.o 			\
					   third_party/gtest/gtest_main.o 	\
					   third_party/gtest/gmock-gtest-all.o

hash_test_SRC = polly/stubs/hash_test.o 	\
				polly/stubs/hash.o 			\
				third_party/gtest/gtest_main.o 	\
				third_party/gtest/gmock-gtest-all.o

string_view_test: $(string_view_test_SRC)
	g++ -std=c++11 -g -Wall -Wextra -o $@ $^ -lpthread

hash_test: $(hash_test_SRC)
	g++ -std=c++11 -g -Wall -Wextra -o $@ $^ -lpthread

%.o: %.cc
	g++ -std=c++11 -g -Wall -Wextra -c -o $@ $^ -Ipolly -Ithird_party/gtest

clean:
	rm -rf string_view_test hash_test
	@(echo "rm *.o"; find . -name "*.o" | xargs -r rm)

.PHONY: string_view_test hash_test
