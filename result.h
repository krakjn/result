#ifndef RESULT_H
#define RESULT_H

#include <concepts>
#include <cstdlib>   // std::abort
#include <functional>
#include <memory>    // std::construct_at, std::destroy_at
#include <type_traits>
#include <utility>

template <typename T, typename E> class Result {
private:
  struct OkType {};
  struct ErrType {};

  union {
    T val;
    E err;
  };
  bool ok;

  Result(T v, OkType) : ok(true) {
    std::construct_at(std::addressof(val), std::move(v));
  }

  Result(E e, ErrType) : ok(false) {
    std::construct_at(std::addressof(err), std::move(e));
  }

public:
  template <typename U>
  static Result<T, E> Ok(U &&v) {
    return Result<T, E>(std::forward<U>(v), OkType{});
  }

  template <typename U>
  static Result<T, E> Err(U &&e) {
    return Result<T, E>(std::forward<U>(e), ErrType{});
  }

  [[nodiscard]] bool is_ok() const { return ok; }
  [[nodiscard]] bool is_err() const { return !ok; }

  T &unwrap() {
    if (!ok) std::abort();
    return val;
  }

  const T &unwrap() const {
    if (!ok) std::abort();
    return val;
  }

  E &unwrap_err() {
    if (ok) std::abort();
    return err;
  }

  const E &unwrap_err() const {
    if (ok) std::abort();
    return err;
  }

  T unwrap_or(T default_value) const { return ok ? val : default_value; }

  T unwrap_or_else(std::function<T()> f) const { return ok ? val : f(); }

  template <typename F>
    requires std::invocable<F, T>
  auto map(F f) const -> Result<std::invoke_result_t<F, T>, E> {
    using U = std::invoke_result_t<F, T>;
    if (ok) return Result<U, E>::Ok(f(val));
    return Result<U, E>::Err(err);
  }

  template <typename F>
    requires std::invocable<F, E>
  auto map_err(F f) const -> Result<T, std::invoke_result_t<F, E>> {
    using E2 = std::invoke_result_t<F, E>;
    if (ok) return Result<T, E2>::Ok(val);
    return Result<T, E2>::Err(f(err));
  }

  template <typename F>
    requires std::invocable<F, T>
  auto and_then(F f) const -> decltype(f(std::declval<T>())) {
    if (ok) return f(val);
    using R = decltype(f(std::declval<T>()));
    return R::Err(err);
  }

  template <typename F>
    requires std::invocable<F, E>
  auto or_else(F f) const -> decltype(f(std::declval<E>())) {
    if (!ok) return f(err);
    using R = decltype(f(std::declval<E>()));
    return R::Ok(val);
  }

  ~Result() {
    if (ok) {
      std::destroy_at(std::addressof(val));
    } else {
      std::destroy_at(std::addressof(err));
    }
  }

  Result(Result &&other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_constructible_v<E>)
      : ok(other.ok) {
    if (ok) {
      std::construct_at(std::addressof(val), std::move(other.val));
    } else {
      std::construct_at(std::addressof(err), std::move(other.err));
    }
  }

  Result(const Result &other) : ok(other.ok) {
    if (ok) {
      std::construct_at(std::addressof(val), other.val);
    } else {
      std::construct_at(std::addressof(err), other.err);
    }
  }

  Result &operator=(Result &&other) noexcept(
      std::is_nothrow_move_assignable_v<T> &&
      std::is_nothrow_move_assignable_v<E>) {
    if (this == &other) return *this;
    this->~Result();
    ok = other.ok;
    if (ok) {
      std::construct_at(std::addressof(val), std::move(other.val));
    } else {
      std::construct_at(std::addressof(err), std::move(other.err));
    }
    return *this;
  }

  Result &operator=(const Result &other) {
    if (this == &other) return *this;
    this->~Result();
    ok = other.ok;
    if (ok) {
      std::construct_at(std::addressof(val), other.val);
    } else {
      std::construct_at(std::addressof(err), other.err);
    }
    return *this;
  }
};

#endif // RESULT_H
