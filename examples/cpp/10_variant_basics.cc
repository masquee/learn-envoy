#include <iostream>
#include <string>
#include <variant>

namespace {

struct TcpProxyConfig {
  std::string cluster;
};

struct HttpConnectionManagerConfig {
  std::string route_name;
};

// FilterConfig 当前只能保存下面两种类型中的一种。
// 可以把它读成：TcpProxyConfig OR HttpConnectionManagerConfig。
using FilterConfig = std::variant<TcpProxyConfig, HttpConnectionManagerConfig>;

void inspect(const FilterConfig &config) {
  // index() 返回当前类型在 variant 类型列表中的位置：
  // 0 表示 TcpProxyConfig，1 表示 HttpConnectionManagerConfig。
  std::cout << "active type index=" << config.index() << '\n';

  // 方法一：先用 holds_alternative<T> 判断，再用 get<T> 取值。
  if (std::holds_alternative<TcpProxyConfig>(config)) {
    // 因为上面已经确认了类型，所以这里调用 get 是安全的。
    const TcpProxyConfig &tcp = std::get<TcpProxyConfig>(config);
    std::cout << "get: tcp_proxy cluster=" << tcp.cluster << '\n';
  }

  // 方法二：get_if<T> 同时完成“判断类型”和“取得指针”。
  // 类型匹配时返回指向内部对象的 pointer；不匹配时返回 nullptr。
  if (const auto *http = std::get_if<HttpConnectionManagerConfig>(&config)) {
    std::cout << "get_if: http_connection_manager route=" << http->route_name
              << '\n';
  }
}

} // namespace

int main() {
  // variant 根据右侧值的类型，选择当前 active alternative。
  FilterConfig config = TcpProxyConfig{"database"};
  inspect(config);

  // 给 variant 赋另一种类型的值，会销毁旧值并构造新值。
  config = HttpConnectionManagerConfig{"local_route"};
  inspect(config);

  // 不要在未确认类型时直接 get：类型不匹配会抛 std::bad_variant_access。
  // Envoy 通常关闭 C++ exceptions，因此更应使用 get_if 或 std::visit。
  // std::get<TcpProxyConfig>(config);  // 当前保存的是 HTTP config，错误。
}
