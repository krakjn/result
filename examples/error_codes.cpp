#include "result.h"
#include <iostream>

enum class ErrorCode { DivisionByZero, InvalidInput };

Result<int, ErrorCode> safe_divide(int a, int b) {
  if (b == 0) {
    return Err(ErrorCode::DivisionByZero);
  }
  return Ok(a / b);
}

const char *describe(ErrorCode code) {
  switch (code) {
  case ErrorCode::DivisionByZero:
    return "division by zero";
  case ErrorCode::InvalidInput:
    return "invalid input";
  }
  return "unknown error";
}

int main() {
  std::cout << "10 / 2: "
            << safe_divide(10, 2).map_err(describe).expect("2 is not zero")
            << "\n";
  std::cout << "10 / 0: " << safe_divide(10, 0).map_err(describe).unwrap_err()
            << "\n";
  return 0;
}
