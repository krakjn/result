# The Demonstrations

Every program is in [`examples/`](https://github.com/ballast-dev/result/tree/main/examples). `ctest` builds each one at its oldest supported standard and runs it. The source and the output below are from that run.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## arithmetic

C++11. `Ok`, `Err`, `expect`, `unwrap_or`.

<!-- capture:arithmetic -->
<div class="demo-file">arithmetic.cpp</div>

```cpp
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
```

```stdout
10 / 2 = 5
10 / 0 = -1 (fallback)
10 / 0 error: division by zero
```
<!-- /capture:arithmetic -->

## and_then

C++11. Chains two steps that can fail.

<!-- capture:and_then -->
<div class="demo-file">and_then.cpp</div>

```cpp
#include "result.h"
#include <iostream>
#include <stdexcept>
#include <string>

Result<int, std::string> parse_number(const std::string &input) {
  try {
    return Ok(std::stoi(input));
  } catch (const std::exception &) {
    return Err("invalid number: " + input);
  }
}

Result<int, std::string> multiply_by_two(int num) { return Ok(num * 2); }

int main() {
  std::cout << "21 * 2 = "
            << parse_number("21").and_then(multiply_by_two).expect("21 parses")
            << "\n";
  std::cout << "abc * 2: "
            << parse_number("abc").and_then(multiply_by_two).unwrap_err()
            << "\n";
  return 0;
}
```

```stdout
21 * 2 = 42
abc * 2: invalid number: abc
```
<!-- /capture:and_then -->

## or_else

C++11. Recovers from an error with a new `Result`.

<!-- capture:or_else -->
<div class="demo-file">or_else.cpp</div>

```cpp
#include "result.h"
#include <iostream>
#include <stdexcept>
#include <string>

Result<int, std::string> parse_number(const std::string &input) {
  try {
    return Ok(std::stoi(input));
  } catch (const std::exception &) {
    return Err("invalid number: " + input);
  }
}

Result<int, std::string> handle_parse_error(const std::string &error) {
  std::cerr << "logged: " << error << "\n";
  return Ok(0);
}

int main() {
  Result<int, std::string> safe =
      parse_number("not_a_number").or_else(handle_parse_error);
  std::cout << "Final value: " << safe.expect("or_else always recovers")
            << "\n";
  return 0;
}
```

```stdout
Final value: 0
```

```stderr
logged: invalid number: not_a_number
```
<!-- /capture:or_else -->

## transform

C++11. `map` on success, and the error passing through it.

<!-- capture:transform -->
<div class="demo-file">transform.cpp</div>

```cpp
#include "result.h"
#include <iostream>
#include <stdexcept>
#include <string>

Result<int, std::string> parse_number(const std::string &input) {
  try {
    return Ok(std::stoi(input));
  } catch (const std::exception &) {
    return Err("invalid number: " + input);
  }
}

int main() {
  std::cout << "42 doubled: "
            << parse_number("42").map([](int n) { return n * 2; }).expect(
                   "42 parses")
            << "\n";
  std::cout << "x doubled: "
            << parse_number("x").map([](int n) { return n * 2; }).unwrap_err()
            << "\n";
  return 0;
}
```

```stdout
42 doubled: 84
x doubled: invalid number: x
```
<!-- /capture:transform -->

## error_codes

C++11. An enum error turned into text at the edge with `map_err`.

<!-- capture:error_codes -->
<div class="demo-file">error_codes.cpp</div>

```cpp
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
```

```stdout
10 / 2: 5
10 / 0: division by zero
```
<!-- /capture:error_codes -->

## file_operation

C++11. `unwrap_or` and `unwrap_or_else` on a missing file.

<!-- capture:file_operation -->
<div class="demo-file">file_operation.cpp</div>

```cpp
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
```

```stdout
exists.txt: File contents here.
missing.txt: Default file contents.
missing.txt: <file not found: missing.txt>
```
<!-- /capture:file_operation -->

## complex_data

C++11. A `std::vector` payload, iterated straight out of a temporary, and `map_or`.

<!-- capture:complex_data -->
<div class="demo-file">complex_data.cpp</div>

```cpp
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
```

```stdout
Generated list: 42 42 42 42 42
Size of generate_list(-1): 0
```
<!-- /capture:complex_data -->

## try

C++11. A parse, validate, compute pipeline using `RESULT_TRY`.

<!-- capture:try -->
<div class="demo-file">try.cpp</div>

```cpp
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
```

```stdout
200 at 25% off: 150
200 at abc% off: error, not a number: 'abc'
200 at 150% off: error, percent out of range: 150
```
<!-- /capture:try -->

## void_result

C++11. `Result<void, E>` for saving a config, with `RESULT_TRY(expr)`, `and_then`, and `map`.

<!-- capture:void_result -->
<div class="demo-file">void_result.cpp</div>

```cpp
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
```

```stdout
wrote settings.ini
save(no_path): no path to write to
validate then write: retries cannot be negative
attempts: 3
```
<!-- /capture:void_result -->

## defaults

C++11. `unwrap_or`, `unwrap_or_default`, `unwrap_or_else`, `map_or`.

<!-- capture:defaults -->
<div class="demo-file">defaults.cpp</div>

```cpp
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
```

```stdout
unwrap_or: 8080
unwrap_or_default: 0
unwrap_or_else: 0
map_or https: open on 443
map_or gopher: closed
```

```stderr
warning: unknown service: gopher
```
<!-- /capture:defaults -->

## move_only

C++11. A `std::unique_ptr` moving through `and_then`, `map`, and `expect`. Any copy would fail to compile.

<!-- capture:move_only -->
<div class="demo-file">move_only.cpp</div>

```cpp
#include "result.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct Connection {
  explicit Connection(std::string h) : host(std::move(h)) {}
  std::string host;
};

using Conn = std::unique_ptr<Connection>;

Result<Conn, std::string> connect(const std::string &host) {
  if (host.empty()) {
    return Err("no host");
  }
  return Ok(Conn(new Connection(host)));
}

Result<Conn, std::string> authenticate(Conn conn) {
  if (conn->host == "evil.example") {
    return Err("refused: " + conn->host);
  }
  return Ok(std::move(conn));
}

int main() {
  Conn conn = connect("db.internal")
                  .and_then(authenticate)
                  .expect("db.internal is trusted");
  std::cout << "connected to " << conn->host << "\n";

  std::size_t length = connect("cache.internal")
                           .map([](Conn c) { return c->host.size(); })
                           .unwrap_or(0);
  std::cout << "cache.internal host length: " << length << "\n";

  std::cout << "evil.example: "
            << connect("evil.example").and_then(authenticate).unwrap_err()
            << "\n";
  return 0;
}
```

```stdout
connected to db.internal
cache.internal host length: 14
evil.example: refused: evil.example
```
<!-- /capture:move_only -->

## optional

C++17. `ok()` and `err()`.

<!-- capture:optional -->
<div class="demo-file">optional.cpp</div>

```cpp
#include "result.h"
#include <iostream>
#include <optional>
#include <string>

Result<int, std::string> parse(const std::string &text) {
  if (text.empty() ||
      text.find_first_not_of("0123456789") != std::string::npos) {
    return Err("not a number: " + text);
  }
  return Ok(std::stoi(text));
}

int main() {
  std::optional<int> value = parse("12").ok();
  std::optional<int> missing = parse("twelve").ok();
  std::optional<std::string> why = parse("twelve").err();

  std::cout << "ok() of 12: " << value.value_or(-1) << "\n";
  std::cout << "ok() of twelve has a value: " << std::boolalpha
            << missing.has_value() << "\n";
  std::cout << "err() of twelve: " << why.value_or("none") << "\n";
  return 0;
}
```

```stdout
ok() of 12: 12
ok() of twelve has a value: false
err() of twelve: not a number: twelve
```
<!-- /capture:optional -->
