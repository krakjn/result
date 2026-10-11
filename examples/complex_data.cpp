#include "result.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

Result<std::vector<int>, std::string> generate_list(int count) {
  if (count < 0) {
    return Err("count cannot be negative");
  }
  return Ok(std::vector<int>(static_cast<std::size_t>(count), 42));
}

int main() {
  std::cout << "Generated list:";
  for (int value : generate_list(5).expect("5 is not negative")) {
    std::cout << " " << value;
  }
  std::cout << "\n";

  std::size_t size = generate_list(-1).map_or(
      std::size_t(0), [](const std::vector<int> &v) { return v.size(); });
  std::cout << "Size of generate_list(-1): " << size << "\n";
  return 0;
}
