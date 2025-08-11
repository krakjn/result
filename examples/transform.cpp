#include "result.h"
#include <format>
#include <iostream>
#include <string>

Result<int, std::string> parse_number(const std::string &input) {
  try {
    return Result<int, std::string>::Ok(std::stoi(input));
  } catch (...) {
    return Result<int, std::string>::Err("Invalid number.");
  }
}

int main() {
  auto number_result = parse_number("42").map([](int num) { return num * 2; });

  if (number_result.is_ok()) {
    std::cout << std::format("Transformed Result: {}\n", number_result.unwrap());
  } else {
    std::cout << std::format("Error: {}\n", number_result.unwrap_err());
  }

  return 0;
}
