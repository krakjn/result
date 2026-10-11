# The Machinery

`Result<T, E>` holds a `T` or an `E`, never a polite mixture of both. They live in a union. Copy and move are there, so you can return one from a function without inventing a third state.

The examples compile as C++20. The header still knows how to construct itself on earlier dialects, using allocators instead of `std::construct_at`. The calls below do not change.

## Ok and Err

```cpp
static Result<T, E> Ok(U&& value);
static Result<T, E> Err(U&& error);
```

These are the only constructors you should be calling. Both forward into the union and set the flag.

```cpp
return Result<int, std::string>::Ok(a / b);
return Result<int, std::string>::Err("division by zero");
```

## is_ok and is_err

```cpp
bool is_ok() const;
bool is_err() const;
```

Ask which side is alive. Both are marked to warn if you ignore them. Ignoring a `Result` is how the bug gets a pension.

## unwrap and unwrap_err

```cpp
T& unwrap();
const T& unwrap() const;
E& unwrap_err();
const E& unwrap_err() const;
```

You get a reference to the side you named. The other side calls `std::abort`. That is the whole error policy: be sure, or stop the process. I have shipped the other policy. It logs a warning and keeps going, and then everyone has a story.

## unwrap_or and unwrap_or_else

```cpp
T unwrap_or(T default_value) const;
T unwrap_or_else(std::function<T()> fallback) const;
```

Both hand back a `T` by value. `unwrap_or` copies the live value, or the default you passed in. `unwrap_or_else` calls the function only on the error path. It takes a `std::function`, which is the expensive way to spell "call this if we are sad." `T` has to be copyable. I am not sorry.

```cpp
int value = divide(10, 0).unwrap_or(-1);
```

## map and map_err

```cpp
auto mapped = result.map([](int n) { return n * 2; });
auto relabeled = result.map_err([](const std::string& err) {
    return std::string("nope: ") + err;
});
```

`map` runs on the value and leaves the error type alone. `map_err` runs on the error and leaves the value type alone. The dead side is forwarded as it is. Neither one will save you from `unwrap`.

## and_then and or_else

`and_then` is for the next fallible step. The function is called only when the value is alive, and it must return a `Result`. On the error path the current error is passed to that result's `Err`.

```cpp
auto chained = parse_number("21").and_then(multiply_by_two);
```

`or_else` is the other door. The function is called only when the error is alive, and it must return a `Result`. On the success path the current value is passed to that result's `Ok`.

```cpp
auto recovered = parse_number("nope").or_else(handle_parse_error);
```

Keep the types compatible with the result you return. The header will not invent a conversion for you, and I am finished inventing those.
