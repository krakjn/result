# The Demonstrations

CMake compiles these seven programs and `ctest` runs them. They all include `result.h` and link nothing else. The binary name is the file name.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/arithmetic
```

## arithmetic

Divide, then survive a zero. `examples/arithmetic.cpp` prints the quotient, and `unwrap_or(-1)` when the denominator is the old enemy.

## and_then

`examples/and_then.cpp` parses `"21"` and multiplies by two, as a second `Result`. The chain is `parse_number("21").and_then(multiply_by_two)`. Forty-two, if the string was a number. An error, if it was a mood.

## or_else

`examples/or_else.cpp` parses `"not_a_number"`, logs the error, and recovers with `Ok(0)`. `or_else` is the apology. It still has to return a `Result`.

## transform

`examples/transform.cpp` is `map`. Parse `"42"`, double it, print `84`. `map` is for when the next step cannot fail and you are tired of pretending it might.

## error_codes

`examples/error_codes.cpp` uses `Result<int, ErrorCode>` with an enum instead of a string. The string happens at the edge, in `error_code_to_string`, where strings belong.

## file_operation

`examples/file_operation.cpp` does not open a file. It checks the name against `"exists.txt"` and otherwise returns `"File not found."`. The interesting line is `unwrap_or("Default file contents.")` on the missing one. I have written the real version. It was longer and worse.

## complex_data

`examples/complex_data.cpp` returns `Result<std::vector<int>, std::string>`. Negative counts are the error. A non-negative count is a vector full of `42`. The header does not care that `T` has a heap inside it. The union still only keeps one side alive.
