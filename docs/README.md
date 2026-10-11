# The Header

`Result<T, E>` is a value or an error, in the Rust shape, for C++. The whole library is `result.h`. Copy it. Include it. There is nothing to link, which is the part I trust. I have watched too many build systems grow a personality.

```cpp
#include "result.h"

Result<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return Result<int, std::string>::Err("division by zero");
    }
    return Result<int, std::string>::Ok(a / b);
}
```

`Ok` and `Err` are static members. They are not free functions wandering around the global namespace, which is how I used to lose a weekend.

The value and the error share a union. A bool remembers which one is alive. The destructor destroys that one. `unwrap` on the wrong one calls `std::abort`, because returning a polite error from the error itself is how afternoons disappear.

The examples are the only translation units. CMake builds them and `ctest` runs them:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`result` is an interface library. Installing it writes `include/result.h` and then stops. The linker stays unemployed.

The [machinery](api.md) is the method list. The [demonstrations](examples.md) are the seven programs.
