#include "result.h"
#include <iostream>
#include <optional>
#include <string>

Result<int, std::string> parse(const std::string &text) {
  if (text.empty() ||
      text.find_first_not_of("0123456789") != std::string::npos) {
    return Err("not a number: " + text);
  }
  return Ok(std::stoi(text));
}

int main() {
  std::optional<int> value = parse("12").ok();
  std::optional<int> missing = parse("twelve").ok();
  std::optional<std::string> why = parse("twelve").err();

  std::cout << "ok() of 12: " << value.value_or(-1) << "\n";
  std::cout << "ok() of twelve has a value: " << std::boolalpha
            << missing.has_value() << "\n";
  std::cout << "err() of twelve: " << why.value_or("none") << "\n";
  return 0;
}
