#include <iostream>
#include <string>

#include "absl/types/optional.h"

namespace {

struct RouteConfig {
  std::string cluster;
  absl::optional<int> timeout_ms; // “没有配置”和“配置为 0”含义不同。
};

int effectiveTimeout(const RouteConfig &route, int default_timeout_ms) {
  return route.timeout_ms.value_or(default_timeout_ms);
}

void printRoute(const RouteConfig &route) {
  std::cout << route.cluster << ": ";
  if (route.timeout_ms.has_value()) {
    std::cout << "explicit timeout=" << *route.timeout_ms << "ms\n";
  } else {
    std::cout << "timeout is inherited\n";
  }
}

} // namespace

int main() {
  RouteConfig inherited{"orders", absl::nullopt};
  RouteConfig explicit_timeout{"payments", 250};

  printRoute(inherited);
  printRoute(explicit_timeout);
  std::cout << "effective orders timeout=" << effectiveTimeout(inherited, 1000)
            << "ms\n";
}
