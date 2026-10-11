// Copyright (c) 2024-2026 Tony B
// SPDX-License-Identifier: MIT

#ifndef RESULT_H
#define RESULT_H

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

// MSVC reports __cplusplus as 199711L unless /Zc:__cplusplus is passed.
#if defined(_MSVC_LANG) && _MSVC_LANG > __cplusplus
#define RESULT_CPP _MSVC_LANG
#else
#define RESULT_CPP __cplusplus
#endif

#if RESULT_CPP >= 201703L
#include <optional>
#define RESULT_HAS_OPTIONAL 1
#define RESULT_NODISCARD [[nodiscard]]
#define RESULT_NODISCARD_CLASS [[nodiscard]]
#else
#define RESULT_HAS_OPTIONAL 0
#if defined(__GNUC__) || defined(__clang__)
#define RESULT_NODISCARD __attribute__((warn_unused_result))
#else
#define RESULT_NODISCARD
#endif
#define RESULT_NODISCARD_CLASS
#endif

// Before C++20 a constant expression cannot construct or destroy a
// non-trivial union member, which is how Result stores its value.
#if RESULT_CPP >= 202002L && defined(__cpp_lib_constexpr_dynamic_alloc)
#define RESULT_HAS_CONSTEXPR 1
#define RESULT_CONSTEXPR20 constexpr
#else
#define RESULT_HAS_CONSTEXPR 0
#define RESULT_CONSTEXPR20
#endif

template <typename T, typename E> class Result;

namespace result_detail {

template <typename T> using decay_t = typename std::decay<T>::type;

template <typename F, typename... A>
using invoke_result_t = decltype(std::declval<F>()(std::declval<A>()...));

template <typename T> struct ok_value { T value; };
template <> struct ok_value<void> {};
template <typename E> struct err_value { E value; };

struct ok_tag {};
struct err_tag {};

template <typename R> struct is_result : std::false_type {};
template <typename T, typename E>
struct is_result<Result<T, E>> : std::true_type {};

[[noreturn]] inline void panic(const char *msg) noexcept {
  std::fputs(msg, stderr);
  std::fputc('\n', stderr);
  std::abort();
}

template <typename U, typename... A>
RESULT_CONSTEXPR20 void construct(U *p, A &&...a) {
#if RESULT_HAS_CONSTEXPR
  std::construct_at(p, std::forward<A>(a)...);
#else
  ::new (static_cast<void *>(p)) U(std::forward<A>(a)...);
#endif
}

// C++ has no unit value, so a function returning void maps to Result<void, E>.
template <typename U, typename E> struct invoke_into {
  template <typename F, typename... A>
  static RESULT_CONSTEXPR20 Result<U, E> ok(F &f, A &&...a) {
    return Result<U, E>::Ok(f(std::forward<A>(a)...));
  }
};

template <typename E> struct invoke_into<void, E> {
  template <typename F, typename... A>
  static RESULT_CONSTEXPR20 Result<void, E> ok(F &f, A &&...a) {
    f(std::forward<A>(a)...);
    return Result<void, E>::Ok();
  }
};

} // namespace result_detail

template <typename T>
RESULT_CONSTEXPR20 result_detail::ok_value<result_detail::decay_t<T>>
Ok(T &&value) {
  return {std::forward<T>(value)};
}

inline RESULT_CONSTEXPR20 result_detail::ok_value<void> Ok() { return {}; }

template <typename E>
RESULT_CONSTEXPR20 result_detail::err_value<result_detail::decay_t<E>>
Err(E &&error) {
  return {std::forward<E>(error)};
}

template <typename T, typename E> class RESULT_NODISCARD_CLASS Result {
  static_assert(!std::is_reference<T>::value,
                "Result<T, E>: T must not be a reference");
  static_assert(!std::is_reference<E>::value && !std::is_void<E>::value,
                "Result<T, E>: E must be an object type");

public:
  using value_type = T;
  using error_type = E;

  template <typename U> static RESULT_CONSTEXPR20 Result Ok(U &&value) {
    return Result(result_detail::ok_tag{}, std::forward<U>(value));
  }

  template <typename U> static RESULT_CONSTEXPR20 Result Err(U &&error) {
    return Result(result_detail::err_tag{}, std::forward<U>(error));
  }

  template <typename U, typename = typename std::enable_if<
                            std::is_constructible<T, U &&>::value>::type>
  RESULT_CONSTEXPR20 Result(result_detail::ok_value<U> &&ok)
      : val_(std::move(ok.value)), ok_(true) {}

  template <typename U, typename = typename std::enable_if<
                            std::is_constructible<T, const U &>::value>::type>
  RESULT_CONSTEXPR20 Result(const result_detail::ok_value<U> &ok)
      : val_(ok.value), ok_(true) {}

  template <typename U, typename = typename std::enable_if<
                            std::is_constructible<E, U &&>::value>::type>
  RESULT_CONSTEXPR20 Result(result_detail::err_value<U> &&err)
      : err_(std::move(err.value)), ok_(false) {}

  template <typename U, typename = typename std::enable_if<
                            std::is_constructible<E, const U &>::value>::type>
  RESULT_CONSTEXPR20 Result(const result_detail::err_value<U> &err)
      : err_(err.value), ok_(false) {}

  RESULT_CONSTEXPR20 Result(const Result &other) : ok_(other.ok_) {
    if (ok_) {
      result_detail::construct(std::addressof(val_), other.val_);
    } else {
      result_detail::construct(std::addressof(err_), other.err_);
    }
  }

  RESULT_CONSTEXPR20 Result(Result &&other) noexcept(
      std::is_nothrow_move_constructible<T>::value &&
      std::is_nothrow_move_constructible<E>::value)
      : ok_(other.ok_) {
    if (ok_) {
      result_detail::construct(std::addressof(val_), std::move(other.val_));
    } else {
      result_detail::construct(std::addressof(err_), std::move(other.err_));
    }
  }

  RESULT_CONSTEXPR20 Result &operator=(const Result &other) {
    if (this == &other) {
      return *this;
    }
    if (ok_ && other.ok_) {
      val_ = other.val_;
    } else if (!ok_ && !other.ok_) {
      err_ = other.err_;
    } else {
      destroy();
      ok_ = other.ok_;
      if (ok_) {
        result_detail::construct(std::addressof(val_), other.val_);
      } else {
        result_detail::construct(std::addressof(err_), other.err_);
      }
    }
    return *this;
  }

  RESULT_CONSTEXPR20 Result &operator=(Result &&other) noexcept(
      std::is_nothrow_move_constructible<T>::value &&
      std::is_nothrow_move_assignable<T>::value &&
      std::is_nothrow_move_constructible<E>::value &&
      std::is_nothrow_move_assignable<E>::value) {
    if (this == &other) {
      return *this;
    }
    if (ok_ && other.ok_) {
      val_ = std::move(other.val_);
    } else if (!ok_ && !other.ok_) {
      err_ = std::move(other.err_);
    } else {
      destroy();
      ok_ = other.ok_;
      if (ok_) {
        result_detail::construct(std::addressof(val_), std::move(other.val_));
      } else {
        result_detail::construct(std::addressof(err_), std::move(other.err_));
      }
    }
    return *this;
  }

  RESULT_CONSTEXPR20 ~Result() { destroy(); }

  RESULT_NODISCARD RESULT_CONSTEXPR20 bool is_ok() const noexcept {
    return ok_;
  }
  RESULT_NODISCARD RESULT_CONSTEXPR20 bool is_err() const noexcept {
    return !ok_;
  }

  RESULT_CONSTEXPR20 T &expect(const char *msg) & {
    if (!ok_) {
      result_detail::panic(msg);
    }
    return val_;
  }
  RESULT_CONSTEXPR20 const T &expect(const char *msg) const & {
    if (!ok_) {
      result_detail::panic(msg);
    }
    return val_;
  }
  RESULT_CONSTEXPR20 T expect(const char *msg) && {
    if (!ok_) {
      result_detail::panic(msg);
    }
    return std::move(val_);
  }

  RESULT_CONSTEXPR20 T &unwrap() & { return expect(unwrap_msg()); }
  RESULT_CONSTEXPR20 const T &unwrap() const & { return expect(unwrap_msg()); }
  RESULT_CONSTEXPR20 T unwrap() && {
    return std::move(*this).expect(unwrap_msg());
  }

  RESULT_CONSTEXPR20 E &expect_err(const char *msg) & {
    if (ok_) {
      result_detail::panic(msg);
    }
    return err_;
  }
  RESULT_CONSTEXPR20 const E &expect_err(const char *msg) const & {
    if (ok_) {
      result_detail::panic(msg);
    }
    return err_;
  }
  RESULT_CONSTEXPR20 E expect_err(const char *msg) && {
    if (ok_) {
      result_detail::panic(msg);
    }
    return std::move(err_);
  }

  RESULT_CONSTEXPR20 E &unwrap_err() & { return expect_err(unwrap_err_msg()); }
  RESULT_CONSTEXPR20 const E &unwrap_err() const & {
    return expect_err(unwrap_err_msg());
  }
  RESULT_CONSTEXPR20 E unwrap_err() && {
    return std::move(*this).expect_err(unwrap_err_msg());
  }

  RESULT_CONSTEXPR20 T unwrap_or(T fallback) const & {
    if (ok_) {
      return val_;
    }
    return fallback;
  }
  RESULT_CONSTEXPR20 T unwrap_or(T fallback) && {
    if (ok_) {
      return std::move(val_);
    }
    return fallback;
  }

  RESULT_CONSTEXPR20 T unwrap_or_default() const & {
    if (ok_) {
      return val_;
    }
    return T();
  }
  RESULT_CONSTEXPR20 T unwrap_or_default() && {
    if (ok_) {
      return std::move(val_);
    }
    return T();
  }

  template <typename F> RESULT_CONSTEXPR20 T unwrap_or_else(F &&f) const & {
    if (ok_) {
      return val_;
    }
    return f(err_);
  }
  template <typename F> RESULT_CONSTEXPR20 T unwrap_or_else(F &&f) && {
    if (ok_) {
      return std::move(val_);
    }
    return f(std::move(err_));
  }

  template <typename F, typename U = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, const T &>>>
  RESULT_CONSTEXPR20 Result<U, E> map(F &&f) const & {
    if (ok_) {
      return result_detail::invoke_into<U, E>::ok(f, val_);
    }
    return Result<U, E>::Err(err_);
  }
  template <typename F, typename U = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, T &&>>>
  RESULT_CONSTEXPR20 Result<U, E> map(F &&f) && {
    if (ok_) {
      return result_detail::invoke_into<U, E>::ok(f, std::move(val_));
    }
    return Result<U, E>::Err(std::move(err_));
  }

  template <typename F, typename E2 = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, const E &>>>
  RESULT_CONSTEXPR20 Result<T, E2> map_err(F &&f) const & {
    if (ok_) {
      return Result<T, E2>::Ok(val_);
    }
    return Result<T, E2>::Err(f(err_));
  }
  template <typename F, typename E2 = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, E &&>>>
  RESULT_CONSTEXPR20 Result<T, E2> map_err(F &&f) && {
    if (ok_) {
      return Result<T, E2>::Ok(std::move(val_));
    }
    return Result<T, E2>::Err(f(std::move(err_)));
  }

  template <typename U, typename F>
  RESULT_CONSTEXPR20 U map_or(U fallback, F &&f) const & {
    if (ok_) {
      return f(val_);
    }
    return fallback;
  }
  template <typename U, typename F>
  RESULT_CONSTEXPR20 U map_or(U fallback, F &&f) && {
    if (ok_) {
      return f(std::move(val_));
    }
    return fallback;
  }

  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, const T &>>>
  RESULT_CONSTEXPR20 R and_then(F &&f) const & {
    static_assert(result_detail::is_result<R>::value,
                  "and_then: the function must return a Result");
    if (ok_) {
      return f(val_);
    }
    return R::Err(err_);
  }
  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, T &&>>>
  RESULT_CONSTEXPR20 R and_then(F &&f) && {
    static_assert(result_detail::is_result<R>::value,
                  "and_then: the function must return a Result");
    if (ok_) {
      return f(std::move(val_));
    }
    return R::Err(std::move(err_));
  }

  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, const E &>>>
  RESULT_CONSTEXPR20 R or_else(F &&f) const & {
    static_assert(result_detail::is_result<R>::value,
                  "or_else: the function must return a Result");
    if (!ok_) {
      return f(err_);
    }
    return R::Ok(val_);
  }
  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, E &&>>>
  RESULT_CONSTEXPR20 R or_else(F &&f) && {
    static_assert(result_detail::is_result<R>::value,
                  "or_else: the function must return a Result");
    if (!ok_) {
      return f(std::move(err_));
    }
    return R::Ok(std::move(val_));
  }

#if RESULT_HAS_OPTIONAL
  RESULT_CONSTEXPR20 std::optional<T> ok() const & {
    if (ok_) {
      return val_;
    }
    return std::nullopt;
  }
  RESULT_CONSTEXPR20 std::optional<T> ok() && {
    if (ok_) {
      return std::move(val_);
    }
    return std::nullopt;
  }

  RESULT_CONSTEXPR20 std::optional<E> err() const & {
    if (!ok_) {
      return err_;
    }
    return std::nullopt;
  }
  RESULT_CONSTEXPR20 std::optional<E> err() && {
    if (!ok_) {
      return std::move(err_);
    }
    return std::nullopt;
  }
#endif

private:
  template <typename... A>
  RESULT_CONSTEXPR20 explicit Result(result_detail::ok_tag, A &&...a)
      : val_(std::forward<A>(a)...), ok_(true) {}

  template <typename... A>
  RESULT_CONSTEXPR20 explicit Result(result_detail::err_tag, A &&...a)
      : err_(std::forward<A>(a)...), ok_(false) {}

  RESULT_CONSTEXPR20 void destroy() noexcept {
    if (ok_) {
      val_.~T();
    } else {
      err_.~E();
    }
  }

  static constexpr const char *unwrap_msg() {
    return "called `Result::unwrap()` on an `Err` value";
  }
  static constexpr const char *unwrap_err_msg() {
    return "called `Result::unwrap_err()` on an `Ok` value";
  }

  union {
    T val_;
    E err_;
  };
  bool ok_;
};

// C++ has no unit type, so Rust's Result<(), E> is this specialization.
template <typename E> class RESULT_NODISCARD_CLASS Result<void, E> {
  static_assert(!std::is_reference<E>::value && !std::is_void<E>::value,
                "Result<void, E>: E must be an object type");

public:
  using value_type = void;
  using error_type = E;

  static RESULT_CONSTEXPR20 Result Ok() {
    return Result(result_detail::ok_tag{});
  }

  template <typename U> static RESULT_CONSTEXPR20 Result Err(U &&error) {
    return Result(result_detail::err_tag{}, std::forward<U>(error));
  }

  RESULT_CONSTEXPR20 Result(result_detail::ok_value<void>)
      : none_(), ok_(true) {}

  template <typename U, typename = typename std::enable_if<
                            std::is_constructible<E, U &&>::value>::type>
  RESULT_CONSTEXPR20 Result(result_detail::err_value<U> &&err)
      : err_(std::move(err.value)), ok_(false) {}

  template <typename U, typename = typename std::enable_if<
                            std::is_constructible<E, const U &>::value>::type>
  RESULT_CONSTEXPR20 Result(const result_detail::err_value<U> &err)
      : err_(err.value), ok_(false) {}

  RESULT_CONSTEXPR20 Result(const Result &other) : none_(), ok_(other.ok_) {
    if (!ok_) {
      result_detail::construct(std::addressof(err_), other.err_);
    }
  }

  RESULT_CONSTEXPR20 Result(Result &&other) noexcept(
      std::is_nothrow_move_constructible<E>::value)
      : none_(), ok_(other.ok_) {
    if (!ok_) {
      result_detail::construct(std::addressof(err_), std::move(other.err_));
    }
  }

  RESULT_CONSTEXPR20 Result &operator=(const Result &other) {
    if (this == &other) {
      return *this;
    }
    if (!ok_ && !other.ok_) {
      err_ = other.err_;
    } else if (!other.ok_) {
      result_detail::construct(std::addressof(err_), other.err_);
      ok_ = false;
    } else if (!ok_) {
      err_.~E();
      result_detail::construct(std::addressof(none_));
      ok_ = true;
    }
    return *this;
  }

  RESULT_CONSTEXPR20 Result &operator=(Result &&other) noexcept(
      std::is_nothrow_move_constructible<E>::value &&
      std::is_nothrow_move_assignable<E>::value) {
    if (this == &other) {
      return *this;
    }
    if (!ok_ && !other.ok_) {
      err_ = std::move(other.err_);
    } else if (!other.ok_) {
      result_detail::construct(std::addressof(err_), std::move(other.err_));
      ok_ = false;
    } else if (!ok_) {
      err_.~E();
      result_detail::construct(std::addressof(none_));
      ok_ = true;
    }
    return *this;
  }

  RESULT_CONSTEXPR20 ~Result() {
    if (!ok_) {
      err_.~E();
    }
  }

  RESULT_NODISCARD RESULT_CONSTEXPR20 bool is_ok() const noexcept {
    return ok_;
  }
  RESULT_NODISCARD RESULT_CONSTEXPR20 bool is_err() const noexcept {
    return !ok_;
  }

  RESULT_CONSTEXPR20 void expect(const char *msg) const {
    if (!ok_) {
      result_detail::panic(msg);
    }
  }
  RESULT_CONSTEXPR20 void unwrap() const {
    expect("called `Result::unwrap()` on an `Err` value");
  }

  RESULT_CONSTEXPR20 E &expect_err(const char *msg) & {
    if (ok_) {
      result_detail::panic(msg);
    }
    return err_;
  }
  RESULT_CONSTEXPR20 const E &expect_err(const char *msg) const & {
    if (ok_) {
      result_detail::panic(msg);
    }
    return err_;
  }
  RESULT_CONSTEXPR20 E expect_err(const char *msg) && {
    if (ok_) {
      result_detail::panic(msg);
    }
    return std::move(err_);
  }

  RESULT_CONSTEXPR20 E &unwrap_err() & { return expect_err(unwrap_err_msg()); }
  RESULT_CONSTEXPR20 const E &unwrap_err() const & {
    return expect_err(unwrap_err_msg());
  }
  RESULT_CONSTEXPR20 E unwrap_err() && {
    return std::move(*this).expect_err(unwrap_err_msg());
  }

  template <typename F, typename U = result_detail::decay_t<
                            result_detail::invoke_result_t<F &>>>
  RESULT_CONSTEXPR20 Result<U, E> map(F &&f) const & {
    if (ok_) {
      return result_detail::invoke_into<U, E>::ok(f);
    }
    return Result<U, E>::Err(err_);
  }
  template <typename F, typename U = result_detail::decay_t<
                            result_detail::invoke_result_t<F &>>>
  RESULT_CONSTEXPR20 Result<U, E> map(F &&f) && {
    if (ok_) {
      return result_detail::invoke_into<U, E>::ok(f);
    }
    return Result<U, E>::Err(std::move(err_));
  }

  template <typename F, typename E2 = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, const E &>>>
  RESULT_CONSTEXPR20 Result<void, E2> map_err(F &&f) const & {
    if (ok_) {
      return Result<void, E2>::Ok();
    }
    return Result<void, E2>::Err(f(err_));
  }
  template <typename F, typename E2 = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, E &&>>>
  RESULT_CONSTEXPR20 Result<void, E2> map_err(F &&f) && {
    if (ok_) {
      return Result<void, E2>::Ok();
    }
    return Result<void, E2>::Err(f(std::move(err_)));
  }

  template <typename U, typename F>
  RESULT_CONSTEXPR20 U map_or(U fallback, F &&f) const {
    if (ok_) {
      return f();
    }
    return fallback;
  }

  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &>>>
  RESULT_CONSTEXPR20 R and_then(F &&f) const & {
    static_assert(result_detail::is_result<R>::value,
                  "and_then: the function must return a Result");
    if (ok_) {
      return f();
    }
    return R::Err(err_);
  }
  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &>>>
  RESULT_CONSTEXPR20 R and_then(F &&f) && {
    static_assert(result_detail::is_result<R>::value,
                  "and_then: the function must return a Result");
    if (ok_) {
      return f();
    }
    return R::Err(std::move(err_));
  }

  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, const E &>>>
  RESULT_CONSTEXPR20 R or_else(F &&f) const & {
    static_assert(result_detail::is_result<R>::value,
                  "or_else: the function must return a Result");
    if (!ok_) {
      return f(err_);
    }
    return R::Ok();
  }
  template <typename F, typename R = result_detail::decay_t<
                            result_detail::invoke_result_t<F &, E &&>>>
  RESULT_CONSTEXPR20 R or_else(F &&f) && {
    static_assert(result_detail::is_result<R>::value,
                  "or_else: the function must return a Result");
    if (!ok_) {
      return f(std::move(err_));
    }
    return R::Ok();
  }

#if RESULT_HAS_OPTIONAL
  RESULT_CONSTEXPR20 std::optional<E> err() const & {
    if (!ok_) {
      return err_;
    }
    return std::nullopt;
  }
  RESULT_CONSTEXPR20 std::optional<E> err() && {
    if (!ok_) {
      return std::move(err_);
    }
    return std::nullopt;
  }
#endif

private:
  RESULT_CONSTEXPR20 explicit Result(result_detail::ok_tag)
      : none_(), ok_(true) {}

  template <typename... A>
  RESULT_CONSTEXPR20 explicit Result(result_detail::err_tag, A &&...a)
      : err_(std::forward<A>(a)...), ok_(false) {}

  static constexpr const char *unwrap_err_msg() {
    return "called `Result::unwrap_err()` on an `Ok` value";
  }

  union {
    char none_;
    E err_;
  };
  bool ok_;
};

template <typename T, typename E>
RESULT_CONSTEXPR20 bool operator==(const Result<T, E> &a,
                                   const Result<T, E> &b) {
  if (a.is_ok() != b.is_ok()) {
    return false;
  }
  return a.is_ok() ? static_cast<bool>(a.unwrap() == b.unwrap())
                   : static_cast<bool>(a.unwrap_err() == b.unwrap_err());
}

template <typename E>
RESULT_CONSTEXPR20 bool operator==(const Result<void, E> &a,
                                   const Result<void, E> &b) {
  if (a.is_ok() != b.is_ok()) {
    return false;
  }
  return a.is_ok() || static_cast<bool>(a.unwrap_err() == b.unwrap_err());
}

template <typename T, typename E>
RESULT_CONSTEXPR20 bool operator!=(const Result<T, E> &a,
                                   const Result<T, E> &b) {
  return !(a == b);
}

// A C++ expression cannot return from its enclosing function, so Rust's `?`
// becomes a statement: RESULT_TRY(var, expr) binds the value, RESULT_TRY(expr)
// discards it. Wrap expr in parentheses if it contains a top-level comma.
#define RESULT_DETAIL_CAT_(a, b) a##b
#define RESULT_DETAIL_CAT(a, b) RESULT_DETAIL_CAT_(a, b)
#define RESULT_DETAIL_TMP RESULT_DETAIL_CAT(result_try_, __LINE__)
// MSVC's traditional preprocessor passes __VA_ARGS__ as one argument without
// this extra expansion.
#define RESULT_DETAIL_EXPAND(x) x
#define RESULT_DETAIL_PICK(_1, _2, name, ...) name

#define RESULT_DETAIL_TRY_BIND(var, expr)                                      \
  auto RESULT_DETAIL_TMP = (expr);                                             \
  if (RESULT_DETAIL_TMP.is_err()) {                                            \
    return ::Err(std::move(RESULT_DETAIL_TMP).unwrap_err());                   \
  }                                                                            \
  auto var = std::move(RESULT_DETAIL_TMP).unwrap()

#define RESULT_DETAIL_TRY_VOID(expr)                                           \
  do {                                                                         \
    auto result_try_tmp_ = (expr);                                             \
    if (result_try_tmp_.is_err()) {                                            \
      return ::Err(std::move(result_try_tmp_).unwrap_err());                   \
    }                                                                          \
  } while (0)

#define RESULT_TRY(...)                                                        \
  RESULT_DETAIL_EXPAND(RESULT_DETAIL_PICK(__VA_ARGS__, RESULT_DETAIL_TRY_BIND, \
                                          RESULT_DETAIL_TRY_VOID,              \
                                          unused)(__VA_ARGS__))

#endif // RESULT_H
