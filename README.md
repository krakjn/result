# Result

<p align="center">
One header. Two outcomes. Nothing to link.<br>
<a href="https://ballast-dev.github.io/result/">Docs</a>
&nbsp;·&nbsp;
<a href="https://ballast-dev.github.io/result/api/">The machinery</a>
&nbsp;·&nbsp;
<a href="https://ballast-dev.github.io/result/examples/">The demonstrations</a>
</p>

A `Result<T, E>` in the Rust shape, for C++11 and up. I have shipped enough build systems to know the header is the safe part.

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

`Ok` and `Err` are free functions. The tag converts into any `Result` that can hold it, so the type is written once, on the function, where it belongs. The value and the error share a union. `unwrap` on the wrong side prints a line and calls `abort`, because a polite error returned from the error type is how I used to lose afternoons.

C++ has no `?`, and nothing in the standard can return from the function around an expression, so early return is a macro:

```cpp
Result<int, std::string> half(int a, int b) {
    RESULT_TRY(q, divide(a, b));
    return Ok(q / 2);
}
```

| Standard | What shows up |
| --- | --- |
| C++11 | The whole type, including `RESULT_TRY` and `Result<void, E>` |
| C++17 | `ok()` and `err()` as `std::optional`, and a warning when a `Result` is discarded |
| C++20 | `constexpr`, so one can sit inside a `static_assert` |

Why each gate exists, and which Rust methods got left on the floor, is written down once in the [docs](https://ballast-dev.github.io/result/).

Copy `result.h` next to whoever has to live with it. Include it. Link nothing. There is no library, which is the feature.

The examples and tests are the only thing CMake builds:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`just build` is the same thing at C++20. `just c11`, `just c14`, `just c17`, and `just c20` each rebuild at that one standard. `just serve` runs the docs locally.

[MIT](LICENSE).
