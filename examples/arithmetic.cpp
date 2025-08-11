#include "result.h"
#include <format>
#include <iostream>
#include <string>

using MyType = Result<int, std::string>;

MyType divide(int a, int b) {
  if (b == 0) {
    return MyType::Err("Division by zero");
  }
  return MyType::Ok(a / b);
}

int main() {
  auto result = divide(10, 2);
  if (result.is_ok()) {
    std::cout << std::format("Result: {}\n", result.unwrap());
  } else {
    std::cout << std::format("Error: {}\n", result.unwrap_err());
  }

  auto error_result = divide(10, 0);
  std::cout << std::format("Handled Error: {}\n", error_result.unwrap_or(-1));

  return 0;
}
