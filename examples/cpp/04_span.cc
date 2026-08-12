#include <array>
#include <iostream>
#include <vector>

#include "absl/types/span.h"

namespace {

// Span<T> 是“借用的一段连续对象”，同时兼容 vector、array 和 C 数组。
// Span<const T> 只读；Span<T> 可以修改原容器中的元素。
int sum(absl::Span<const int> values) {
  int result = 0;
  for (int value : values) {
    result += value;
  }
  return result;
}

void multiply(absl::Span<int> values, int factor) {
  for (int &value : values) {
    value *= factor;
  }
}

} // namespace

int main() {
  std::vector<int> dynamic_values{1, 2, 3};
  std::array<int, 2> fixed_values{4, 5};

  // mutable Span 显式创建，清楚表达“这个调用可能修改原容器”。
  multiply(absl::MakeSpan(dynamic_values), 10);
  std::cout << "vector sum=" << sum(absl::MakeConstSpan(dynamic_values))
            << '\n';
  std::cout << "array sum=" << sum(absl::MakeConstSpan(fixed_values)) << '\n';
}
