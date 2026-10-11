#include "result.h"
#include <iostream>
#include <string>

Result<std::string, std::string> read_file(const std::string &filename) {
  if (filename == "exists.txt") {
    return Ok("File contents here.");
  }
  return Err("file not found: " + filename);
}

int main() {
  std::cout << "exists.txt: " << read_file("exists.txt").expect("it exists")
            << "\n";
  std::cout << "missing.txt: "
            << read_file("missing.txt").unwrap_or("Default file contents.")
            << "\n";
  std::cout << "missing.txt: "
            << read_file("missing.txt").unwrap_or_else(
                   [](const std::string &err) { return "<" + err + ">"; })
            << "\n";
  return 0;
}
