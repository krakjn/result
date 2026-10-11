#include "result.h"
#include <iostream>
#include <string>

Result<int, std::string> divide(int a, int b) {
  if (b == 0) {
    return Err("division by zero");
  }
  return Ok(a / b);
}

int main() {
  std::cout << "10 / 2 = " << divide(10, 2).expect("2 is not zero") << "\n";
  std::cout << "10 / 0 = " << divide(10, 0).unwrap_or(-1) << " (fallback)\n";
  std::cout << "10 / 0 error: " << divide(10, 0).unwrap_err() << "\n";
  return 0;
}
