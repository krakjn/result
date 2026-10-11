#include "result.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct Connection {
  explicit Connection(std::string h) : host(std::move(h)) {}
  std::string host;
};

using Conn = std::unique_ptr<Connection>;

Result<Conn, std::string> connect(const std::string &host) {
  if (host.empty()) {
    return Err("no host");
  }
  return Ok(Conn(new Connection(host)));
}

Result<Conn, std::string> authenticate(Conn conn) {
  if (conn->host == "evil.example") {
    return Err("refused: " + conn->host);
  }
  return Ok(std::move(conn));
}

int main() {
  Conn conn = connect("db.internal")
                  .and_then(authenticate)
                  .expect("db.internal is trusted");
  std::cout << "connected to " << conn->host << "\n";

  std::size_t length = connect("cache.internal")
                           .map([](Conn c) { return c->host.size(); })
                           .unwrap_or(0);
  std::cout << "cache.internal host length: " << length << "\n";

  std::cout << "evil.example: "
            << connect("evil.example").and_then(authenticate).unwrap_err()
            << "\n";
  return 0;
}
