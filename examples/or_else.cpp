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

Result<int, std::string> handle_parse_error(const std::string &error) {
  std::cerr << "logged: " << error << "\n";
  return Ok(0);
}

int main() {
  Result<int, std::string> safe =
      parse_number("not_a_number").or_else(handle_parse_error);
  std::cout << "Final value: " << safe.expect("or_else always recovers")
            << "\n";
  return 0;
}
