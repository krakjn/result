# Result

A `Result<T, E>` for C++. One header. I have shipped enough build systems to know the header is the safe part.

```cpp
#include "result.h"

Result<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return Result<int, std::string>::Err("division by zero");
    }
    return Result<int, std::string>::Ok(a / b);
}
```

Copy `result.h` next to whoever has to live with it. Include it. Do not link anything. There is no library, which is the feature. The value and the error share a union, and `unwrap` on the wrong one calls `abort`, because returning a polite error from the error type is how I used to spend afternoons.

The examples are the only thing CMake builds:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The rest of the machinery is in the [docs](https://ballast-dev.github.io/result/).

MIT.
