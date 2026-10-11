#include "result.h"
#include <iostream>
#include <string>

struct Config {
  std::string path;
  int retries;
};

Result<void, std::string> validate(const Config &config) {
  if (config.retries < 0) {
    return Err("retries cannot be negative");
  }
  return Ok();
}

Result<void, std::string> write(const Config &config) {
  if (config.path.empty()) {
    return Err("no path to write to");
  }
  std::cout << "wrote " << config.path << "\n";
  return Ok();
}

Result<void, std::string> save(const Config &config) {
  RESULT_TRY(validate(config));
  RESULT_TRY(write(config));
  return Ok();
}

int main() {
  Config good = {"settings.ini", 3};
  save(good).expect("a valid config saves");

  Config no_path = {"", 3};
  std::cout << "save(no_path): " << save(no_path).unwrap_err() << "\n";

  Config negative = {"settings.ini", -1};
  Result<void, std::string> chained =
      validate(negative).and_then([&negative] { return write(negative); });
  std::cout << "validate then write: "
            << (chained.is_ok() ? std::string("ok") : chained.unwrap_err())
            << "\n";

  int attempts = validate(good).map([&good] { return good.retries; }).unwrap();
  std::cout << "attempts: " << attempts << "\n";
  return 0;
}
