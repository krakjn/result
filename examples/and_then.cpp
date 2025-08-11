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

Result<int, std::string> multiply_by_two(int num) {
  return Result<int, std::string>::Ok(num * 2);
}

int main() {
  auto chained_result = parse_number("21").and_then(multiply_by_two);

  if (chained_result.is_ok()) {
    std::cout << std::format("Chained Result: {}\n", chained_result.unwrap());
  } else {
    std::cout << std::format("Error: {}\n", chained_result.unwrap_err());
  }

  return 0;
}
