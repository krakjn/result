#include "result.h"

#include <cstdio>
#include <memory>
#include <string>

static int failures = 0;

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__,    \
                   #cond);                                                     \
      ++failures;                                                              \
    }                                                                          \
  } while (0)

typedef Result<int, std::string> IntResult;
typedef Result<void, std::string> Unit;
typedef Result<std::unique_ptr<int>, std::string> Owned;

static IntResult parse(const std::string &text) {
  if (text.empty() ||
      text.find_first_not_of("0123456789") != std::string::npos) {
    return Err("not a number: " + text);
  }
  return Ok(std::stoi(text));
}

static Unit check_positive(int value) {
  if (value <= 0) {
    return Err("not positive");
  }
  return Ok();
}

static Owned make_owned(int value) {
  return Ok(std::unique_ptr<int>(new int(value)));
}

static IntResult add_parsed(const std::string &a, const std::string &b) {
  RESULT_TRY(x, parse(a));
  RESULT_TRY(y, parse(b));
  return Ok(x + y);
}

static IntResult positive(const std::string &text) {
  RESULT_TRY(value, parse(text));
  RESULT_TRY(check_positive(value));
  return Ok(value);
}

static Unit all_positive(int a, int b) {
  RESULT_TRY(check_positive(a));
  RESULT_TRY(check_positive(b));
  return Ok();
}

static void construction() {
  IntResult a = Ok(1);
  IntResult b = IntResult::Ok(1);
  IntResult c = Err("boom");
  IntResult d = IntResult::Err(std::string("boom"));
  CHECK(a.is_ok() && !a.is_err());
  CHECK(c.is_err() && !c.is_ok());
  CHECK(a == b);
  CHECK(c == d);
  CHECK(a != c);

  Result<int, int> same_ok = Ok(7);
  Result<int, int> same_err = Err(7);
  CHECK(same_ok.is_ok());
  CHECK(same_err.is_err());
  CHECK(same_ok != same_err);
}

static void extracting() {
  IntResult good = parse("41");
  good.unwrap() += 1;
  CHECK(good.unwrap() == 42);
  CHECK(good.expect("parses") == 42);
  CHECK(parse("5").unwrap() == 5);

  IntResult bad = parse("x");
  bad.unwrap_err() += "!";
  CHECK(bad.unwrap_err() == "not a number: x!");
  CHECK(bad.expect_err("does not parse") == "not a number: x!");
  CHECK(parse("x").unwrap_err() == "not a number: x");

  CHECK(parse("x").unwrap_or(-1) == -1);
  CHECK(parse("3").unwrap_or(-1) == 3);
  CHECK(good.unwrap_or(0) == 42);
  CHECK(parse("x").unwrap_or_default() == 0);
  CHECK(parse("x").unwrap_or_else(
            [](const std::string &err) { return static_cast<int>(err.size()); }) ==
        15);
  CHECK(good.unwrap_or_else([](const std::string &) { return 0; }) == 42);
}

static void transforming() {
  CHECK(parse("21").map([](int n) { return n * 2; }).unwrap() == 42);
  CHECK(parse("x").map([](int n) { return n * 2; }).is_err());

  int seen = 0;
  Unit mapped_to_void = parse("9").map([&seen](int n) { seen = n; });
  CHECK(mapped_to_void.is_ok());
  CHECK(seen == 9);

  Result<int, std::size_t> err_len =
      parse("abc").map_err([](const std::string &err) { return err.size(); });
  CHECK(err_len.unwrap_err() == 17);
  CHECK(parse("1").map_err([](const std::string &err) { return err.size(); })
            .unwrap() == 1);

  CHECK(parse("4").map_or(0, [](int n) { return n * 10; }) == 40);
  CHECK(parse("x").map_or(0, [](int n) { return n * 10; }) == 0);

  CHECK(parse("10").and_then([](int n) -> IntResult {
                      if (n > 5) {
                        return Ok(n - 5);
                      }
                      return Err("too small");
                    }).unwrap() == 5);
  CHECK(parse("x").and_then([](int n) { return IntResult(Ok(n)); })
            .unwrap_err() == "not a number: x");

  CHECK(parse("x").or_else([](const std::string &) { return IntResult(Ok(0)); })
            .unwrap() == 0);
  CHECK(parse("2").or_else([](const std::string &) { return IntResult(Ok(0)); })
            .unwrap() == 2);

  Result<int, int> recoded =
      parse("x").or_else([](const std::string &err) {
        return Result<int, int>(Err(static_cast<int>(err.size())));
      });
  CHECK(recoded.unwrap_err() == 15);
}

static void moving() {
  std::unique_ptr<int> p = make_owned(3).unwrap();
  CHECK(*p == 3);

  std::unique_ptr<int> q =
      make_owned(4)
          .map([](std::unique_ptr<int> v) -> std::unique_ptr<int> {
            *v *= 2;
            return v;
          })
          .expect("owned");
  CHECK(*q == 8);

  std::unique_ptr<int> r =
      make_owned(5).and_then([](std::unique_ptr<int> v) -> Owned {
        return Ok(std::move(v));
      }).unwrap();
  CHECK(*r == 5);

  Owned held = make_owned(6);
  CHECK(*held.unwrap() == 6);
  Owned moved = std::move(held);
  CHECK(*moved.unwrap() == 6);
}

static void assigning() {
  IntResult a = Ok(1);
  IntResult b = Err("x");
  a = b;
  CHECK(a.is_err() && a.unwrap_err() == "x");
  b = Ok(2);
  CHECK(b.is_ok() && b.unwrap() == 2);
  a = std::move(b);
  CHECK(a.unwrap() == 2);
  a = IntResult(Ok(3));
  CHECK(a.unwrap() == 3);

  Unit u = Ok();
  Unit v = Err("y");
  u = v;
  CHECK(u.is_err());
  v = Ok();
  CHECK(v.is_ok());
  u = std::move(v);
  CHECK(u.is_ok());
}

static void unit() {
  CHECK(check_positive(1).is_ok());
  check_positive(1).unwrap();
  check_positive(1).expect("1 is positive");
  CHECK(check_positive(0).unwrap_err() == "not positive");
  CHECK(check_positive(0).expect_err("0 is not positive") == "not positive");

  CHECK(check_positive(1).map([] { return 5; }).unwrap() == 5);
  CHECK(check_positive(0).map([] { return 5; }).is_err());
  CHECK(check_positive(0)
            .map_err([](const std::string &err) { return err.size(); })
            .unwrap_err() == 12);
  CHECK(check_positive(1).map_or(0, [] { return 1; }) == 1);
  CHECK(check_positive(0).map_or(0, [] { return 1; }) == 0);
  CHECK(check_positive(1).and_then([] { return parse("8"); }).unwrap() == 8);
  CHECK(check_positive(0)
            .or_else([](const std::string &) { return Unit(Ok()); })
            .is_ok());
  CHECK(check_positive(1) == Unit(Ok()));
  CHECK(check_positive(0) == Unit(Err("not positive")));
  CHECK(check_positive(0) != Unit(Ok()));
}

static void trying() {
  CHECK(add_parsed("2", "3").unwrap() == 5);
  CHECK(add_parsed("2", "x").unwrap_err() == "not a number: x");
  CHECK(positive("4").unwrap() == 4);
  CHECK(positive("0").unwrap_err() == "not positive");
  CHECK(all_positive(1, 2).is_ok());
  CHECK(all_positive(1, -2).unwrap_err() == "not positive");
}

#if RESULT_HAS_OPTIONAL
static void optional() {
  CHECK(parse("7").ok() == std::optional<int>(7));
  CHECK(!parse("x").ok().has_value());
  CHECK(parse("x").err() == std::optional<std::string>("not a number: x"));
  CHECK(!parse("7").err().has_value());
  CHECK(*make_owned(2).ok().value() == 2);
  CHECK(check_positive(0).err() == std::optional<std::string>("not positive"));
  CHECK(!check_positive(1).err().has_value());
}
#endif

#if RESULT_HAS_CONSTEXPR
constexpr int doubled_or_zero(int value) {
  Result<int, int> r = value >= 0 ? Result<int, int>(Ok(value))
                                  : Result<int, int>(Err(value));
  return r.map([](int n) { return n * 2; }).unwrap_or(0);
}

constexpr bool assigns_across_sides() {
  Result<int, int> a = Ok(1);
  Result<int, int> b = a;
  b = Result<int, int>(Err(2));
  a = b;
  return a.unwrap_err() == 2 && b == Result<int, int>(Err(2));
}

static_assert(doubled_or_zero(21) == 42, "constexpr map");
static_assert(doubled_or_zero(-1) == 0, "constexpr unwrap_or");
static_assert(assigns_across_sides(), "constexpr assignment");
#endif

int main() {
  construction();
  extracting();
  transforming();
  moving();
  assigning();
  unit();
  trying();
#if RESULT_HAS_OPTIONAL
  optional();
#endif
  if (failures != 0) {
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
  }
  std::printf("all checks passed (C++ %ld)\n", static_cast<long>(RESULT_CPP));
  return 0;
}
