# The Demonstrations

Every program is in [`examples/`](https://github.com/ballast-dev/result/tree/main/examples). `ctest` builds each one at its oldest supported standard and runs it.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## arithmetic

C++11. `Ok`, `Err`, `expect`, `unwrap_or`.

## and_then

C++11. Chains two steps that can fail.

## or_else

C++11. Recovers from an error with a new `Result`.

## transform

C++11. `map` on success, and the error passing through it.

## error_codes

C++11. An enum error turned into text at the edge with `map_err`.

## file_operation

C++11. `unwrap_or` and `unwrap_or_else` on a missing file.

## complex_data

C++11. A `std::vector` payload, iterated straight out of a temporary, and `map_or`.

## try

C++11. A parse, validate, compute pipeline using `RESULT_TRY`.

## void_result

C++11. `Result<void, E>` for saving a config, with `RESULT_TRY(expr)`, `and_then`, and `map`.

## defaults

C++11. `unwrap_or`, `unwrap_or_default`, `unwrap_or_else`, `map_or`.

## move_only

C++11. A `std::unique_ptr` moving through `and_then`, `map`, and `expect`. Any copy would fail to compile.

## optional

C++17. `ok()` and `err()`.
