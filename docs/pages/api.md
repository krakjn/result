# The Machinery

Rust's `Result`, trimmed to what C++ needs. One header, C++11 and up.

## Construct

```cpp
return Ok(value);               // any Result<T, E> whose T accepts value
return Err(error);              // any Result<T, E> whose E accepts error
return Ok();                    // Result<void, E>
Result<T, E>::Ok(value);        // the explicit spelling still works
```

`Result<void, E>` stands in for Rust's `Result<(), E>`, because C++ has no unit value. See [void_result](examples.md#void_result).

## Query and extract

```cpp
r.is_ok();  r.is_err();
r.unwrap();       r.expect("why it can't fail");
r.unwrap_err();   r.expect_err("why it can't succeed");
r.unwrap_or(fallback);  r.unwrap_or_default();  r.unwrap_or_else(f);  // f(E) -> T
```

On a named `Result`, `unwrap` and `expect` return a reference. On a temporary they move the value out, so `make().unwrap()` works with move-only types and never leaves a dangling reference. See [move_only](examples.md#move_only).

A panic prints its message to `stderr`, then calls `std::abort`. It never prints the payload: C++ has no `Debug` trait to format an arbitrary `E`.

## Transform

```cpp
r.map(f);          // f(T) -> U, gives Result<U, E>. f returning void gives Result<void, E>.
r.map_err(f);      // f(E) -> E2
r.map_or(fallback, f);
r.and_then(f);     // f(T) -> Result<U, E>
r.or_else(f);      // f(E) -> Result<T, E2>
```

## Early return

```cpp
RESULT_TRY(value, parse(text));   // let value = parse(text)?;
RESULT_TRY(save(config));         // save(config)?;
```

It is a macro and a statement, because no standard C++ expression can return from its enclosing function. The error converts into the caller's `Result`. If `expr` has a top-level comma, wrap it in parentheses. See [try](examples.md#try).

## Optional and compare

```cpp
r.ok();    // std::optional<T>, C++17
r.err();   // std::optional<E>, C++17
a == b;  a != b;
```

## Standards

| Standard | Adds |
| --- | --- |
| C++11 | everything above, except below |
| C++17 | `ok()`, `err()`, and `[[nodiscard]]` on the type itself |
| C++20 | `constexpr`, so a `Result` can be used in a `static_assert` |

Why the gates:

- `ok()` / `err()` need C++17, because `std::optional` arrived then.
- Before C++17, only `is_ok` / `is_err` warn when ignored: attributes on a class are C++17.
- `constexpr` needs C++20: before that, a constant expression cannot construct or destroy a non-trivial union member, and that is where the value lives.
- MSVC reports `__cplusplus` as `199711L` by default, so the header reads `_MSVC_LANG` instead.

## Left out

- `is_ok_and` / `is_err_and`: `r.is_ok() && f(r.unwrap())` is one line.
- `inspect` / `inspect_err`: check and log in place.
- `and` / `or`: reserved tokens in C++, and `and_then` / `or_else` cover them.
- `map_or_else`: `map_or` or an `if`.
- `flatten`, `transpose`: rare even in Rust.
- `swap`: `std::swap` already works.
- `iter`, `as_ref`, `as_mut`, `copied`, `cloned`, `unwrap_unchecked`: references, copies, and `*` already do that job.
- An expression form of `?`: it needs a GCC/Clang extension.
