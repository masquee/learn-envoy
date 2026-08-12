#include <iostream>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace {

struct Endpoint {
  std::string host;
  int port;
};

// StatusOr<T> 表示两种互斥结果：成功时持有 T，失败时持有 Status。
absl::StatusOr<int> parsePort(absl::string_view text) {
  int port = 0;
  if (!absl::SimpleAtoi(text, &port)) {
    return absl::InvalidArgumentError(
        absl::StrCat("port is not an integer: ", text));
  }
  if (port < 1 || port > 65535) {
    return absl::OutOfRangeError(
        absl::StrCat("port must be in [1, 65535], got ", port));
  }
  return port; // T 会被隐式包装为成功的 StatusOr<T>。
}

absl::StatusOr<Endpoint> parseEndpoint(absl::string_view host,
                                       absl::string_view port_text) {
  absl::StatusOr<int> port = parsePort(port_text);
  if (!port.ok()) {
    // 给底层错误补充上下文，再向上传播。Envoy 代码里很常见。
    return absl::Status(
        port.status().code(),
        absl::StrCat("invalid endpoint: ", port.status().message()));
  }
  return Endpoint{std::string(host), *port}; // *port 取出成功值。
}

void printResult(absl::string_view host, absl::string_view port) {
  absl::StatusOr<Endpoint> endpoint = parseEndpoint(host, port);
  if (!endpoint.ok()) {
    std::cout << "ERROR code=" << endpoint.status().code() << " message=\""
              << endpoint.status().message() << "\"\n";
    return;
  }

  // operator-> 直接访问 StatusOr 内对象；前提是已检查 ok()。
  std::cout << "OK " << endpoint->host << ':' << endpoint->port << '\n';
}

} // namespace

int main() {
  printResult("127.0.0.1", "10000");
  printResult("127.0.0.1", "abc");
  printResult("127.0.0.1", "70000");
}
