# Result

A `Result<T, E>` for C++. One header. I have shipped enough build systems to know the header is the safe part.

```cpp
#include "result.h"

Result<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return Err("division by zero");
    }
    return Ok(a / b);
}

int half_of_ten = divide(10, 2).expect("2 is not zero");
```

Copy `result.h` next to whoever has to live with it. Include it. Do not link anything. There is no library, which is the feature. It compiles back to C++11, because some of us still maintain things. The value and the error share a union, and `unwrap` on the wrong one calls `abort`, because returning a polite error from the error type is how I used to spend afternoons.

The examples and tests are the only thing CMake builds:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The rest of the machinery is in the [docs](https://ballast-dev.github.io/result/). `docs/build.sh serve` runs them locally.

MIT.
