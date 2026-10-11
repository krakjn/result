# The Header

`Result<T, E>` is a value or an error, in the Rust shape, for C++11 and up. The whole library is `result.h`. Copy it. Include it. There is nothing to link, which is the part I trust. I have watched too many build systems grow a personality.

```cpp
#include "result.h"

Result<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return Err("division by zero");
    }
    return Ok(a / b);
}

Result<int, std::string> halve_quotient(int a, int b) {
    RESULT_TRY(q, divide(a, b));
    return Ok(q / 2);
}
```

`Ok` and `Err` are free functions that return small tags. A tag converts to any `Result` that can hold it, so you never spell the type twice.

The value and the error share a union, and a bool remembers which one is alive. `unwrap` on the wrong one calls `std::abort`, because returning a polite error from the error itself is how afternoons disappear.

The examples and tests are the only translation units. CMake builds each example at its oldest supported standard, and the tests at C++11, 14, 17, and 20:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`result` is an interface library. Installing it writes `include/result.h` and then stops. The linker stays unemployed.

The [machinery](api.md) is the method list. The [demonstrations](examples.md) are the programs.
