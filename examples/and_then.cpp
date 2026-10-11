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

Result<int, std::string> multiply_by_two(int num) { return Ok(num * 2); }

int main() {
  std::cout << "21 * 2 = "
            << parse_number("21").and_then(multiply_by_two).expect("21 parses")
            << "\n";
  std::cout << "abc * 2: "
            << parse_number("abc").and_then(multiply_by_two).unwrap_err()
            << "\n";
  return 0;
}
