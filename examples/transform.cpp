#include "result.h"
#include <iostream>
#include <stdexcept>
#include <string>

Result<int, std::string> parse_number(const std::string &input) {
  try {
    return Ok(std::stoi(input));
  } catch (const std::exception &) {
    return Err("invalid number: " + input);
  }
}

int main() {
  std::cout << "42 doubled: "
            << parse_number("42").map([](int n) { return n * 2; }).expect(
                   "42 parses")
            << "\n";
  std::cout << "x doubled: "
            << parse_number("x").map([](int n) { return n * 2; }).unwrap_err()
            << "\n";
  return 0;
}
