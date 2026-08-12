#include <iostream>

#include "absl/container/inlined_vector.h"

int main() {
  // 常见元素数不超过 4 时，元素通常直接放在对象内部，避免一次 heap allocation。
  // 超过 4 仍然能自动扩容，API 与 vector 相近。
  absl::InlinedVector<int, 4> retry_delays_ms{10, 25, 50};
  retry_delays_ms.push_back(100);
  retry_delays_ms.push_back(250);

  for (int delay : retry_delays_ms) {
    std::cout << delay << "ms ";
  }
  std::cout << '\n';
}
