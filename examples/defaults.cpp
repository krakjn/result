#include "result.h"
#include <iostream>
#include <string>

Result<int, std::string> lookup_port(const std::string &service) {
  if (service == "http") {
    return Ok(80);
  }
  if (service == "https") {
    return Ok(443);
  }
  return Err("unknown service: " + service);
}

std::string describe(int port) { return "open on " + std::to_string(port); }

int main() {
  std::cout << "unwrap_or: " << lookup_port("gopher").unwrap_or(8080) << "\n";
  std::cout << "unwrap_or_default: "
            << lookup_port("gopher").unwrap_or_default() << "\n";
  std::cout << "unwrap_or_else: "
            << lookup_port("gopher").unwrap_or_else(
                   [](const std::string &err) -> int {
                     std::cerr << "warning: " << err << "\n";
                     return 0;
                   })
            << "\n";
  std::cout << "map_or https: "
            << lookup_port("https").map_or(std::string("closed"), describe)
            << "\n";
  std::cout << "map_or gopher: "
            << lookup_port("gopher").map_or(std::string("closed"), describe)
            << "\n";
  return 0;
}
