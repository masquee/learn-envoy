#include <iostream>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"

namespace {

struct ClusterStats {
  int requests = 0;
  int failures = 0;
};

} // namespace

int main() {
  absl::flat_hash_map<std::string, ClusterStats> stats;

  // try_emplace 只在 key 不存在时构造 value。
  stats.try_emplace("orders", ClusterStats{});
  stats["orders"].requests++;
  stats["orders"].requests++;
  stats["orders"].failures++;

  // Abseil 容器支持 heterogeneous lookup：无需临时构造 std::string。
  const absl::string_view cluster = "orders";
  const auto it = stats.find(cluster);
  if (it != stats.end()) {
    std::cout << cluster << " requests=" << it->second.requests
              << " failures=" << it->second.failures << '\n';
  }

  // 不要依赖 flat_hash_map 的遍历顺序；它不是稳定 API。
}
