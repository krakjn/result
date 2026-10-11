#include "result.h"
#include <iostream>
#include <string>

Result<int, std::string> parse(const std::string &text) {
  if (text.empty() ||
      text.find_first_not_of("0123456789") != std::string::npos) {
    return Err("not a number: '" + text + "'");
  }
  return Ok(std::stoi(text));
}

Result<int, std::string> validate_percent(int value) {
  if (value > 100) {
    return Err("percent out of range: " + std::to_string(value));
  }
  return Ok(value);
}

Result<int, std::string> discounted(const std::string &price,
                                    const std::string &percent) {
  RESULT_TRY(amount, parse(price));
  RESULT_TRY(raw, parse(percent));
  RESULT_TRY(pct, validate_percent(raw));
  return Ok(amount - amount * pct / 100);
}

void report(const std::string &price, const std::string &percent) {
  Result<int, std::string> result = discounted(price, percent);
  std::cout << price << " at " << percent << "% off: ";
  if (result.is_ok()) {
    std::cout << result.unwrap() << "\n";
  } else {
    std::cout << "error, " << result.unwrap_err() << "\n";
  }
}

int main() {
  report("200", "25");
  report("200", "abc");
  report("200", "150");
  return 0;
}
