#include <iostream>
#include <utility>

#include "absl/cleanup/cleanup.h"

namespace {

void handleRequest(bool hand_off) {
  std::cout << "acquire request slot\n";

  // 离开当前 scope 时自动执行，正常 return 和异常路径都不会漏掉清理。
  absl::Cleanup release_slot = [] { std::cout << "release request slot\n"; };

  if (hand_off) {
    std::cout << "ownership handed off\n";
    std::move(release_slot).Cancel(); // 新 owner 将负责清理。
    return;
  }

  std::cout << "request handled locally\n";
}

} // namespace

int main() {
  handleRequest(false);
  handleRequest(true);
}
