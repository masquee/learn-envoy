#include <iostream>
#include <string>

#include "absl/strings/match.h"
#include "absl/strings/string_view.h"

namespace {

// string_view 只是“借用”一段字符，不复制也不拥有内存。
absl::string_view removeScheme(absl::string_view address) {
  constexpr absl::string_view kHttp = "http://";
  constexpr absl::string_view kHttps = "https://";
  if (absl::StartsWith(address, kHttps)) {
    address.remove_prefix(kHttps.size());
  } else if (absl::StartsWith(address, kHttp)) {
    address.remove_prefix(kHttp.size());
  }
  return address;
}

class Cluster {
public:
  explicit Cluster(absl::string_view name) : name_(name) {}

  absl::string_view name() const { return name_; }

private:
  // 成员需要长期保存，所以复制为 owning string，不能存借用临时对象的 view。
  std::string name_;
};

} // namespace

int main() {
  const std::string url = "https://service.example/path";
  const absl::string_view without_scheme = removeScheme(url);
  std::cout << without_scheme << '\n';

  Cluster cluster("orders");
  std::cout << "cluster=" << cluster.name() << '\n';

  // 危险示例（不要这样做）：
  // absl::string_view dangling = std::string("temporary");
  // 临时 string 在这一行结束时销毁，dangling 随即失效。
}
