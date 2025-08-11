#include "result.h"
#include <format>
#include <iostream>
#include <string>
#include <vector>

Result<std::vector<int>, std::string> generate_list(int count) {
  if (count < 0) {
    return Result<std::vector<int>, std::string>::Err(
        "Count cannot be negative.");
  }
  return Result<std::vector<int>, std::string>::Ok(std::vector<int>(count, 42));
}

int main() {
  auto list_result = generate_list(5);
  if (list_result.is_ok()) {
    std::cout << "Generated list: ";
    for (const auto &val : list_result.unwrap()) {
      std::cout << std::format("{} ", val);
    }
    std::cout << '\n';
  } else {
    std::cout << std::format("Error: {}\n", list_result.unwrap_err());
  }

  return 0;
}
