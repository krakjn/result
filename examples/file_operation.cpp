#include "result.h"
#include <format>
#include <iostream>
#include <string>

Result<std::string, std::string> read_file(const std::string &filename) {
  if (filename == "exists.txt") {
    return Result<std::string, std::string>::Ok("File contents here.");
  }
  return Result<std::string, std::string>::Err("File not found.");
}

int main() {
  auto file_result = read_file("exists.txt");
  if (file_result.is_ok()) {
    std::cout << std::format("File read successfully: {}\n", file_result.unwrap());
  } else {
    std::cout << std::format("Error: {}\n", file_result.unwrap_err());
  }

  auto non_existent =
      read_file("missing.txt").unwrap_or("Default file contents.");
  std::cout << std::format("Fallback: {}\n", non_existent);

  return 0;
}
