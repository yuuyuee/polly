#include <iostream>
#include <string>
#include "stubs/strings/string_view.h"
#include "stubs/base/macros.h"

int main() {
  std::string d("hello world");
  polly::string_view s(d);
  std::cout << s.size() << std::endl;
  std::cout.write(s.data(), s.size()) << std::endl;
  std::cout << POLLY_ARRAY_SIZE("123") << std::endl;
  return 0;
}