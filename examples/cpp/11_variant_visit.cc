#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace {

struct TcpProxyConfig {
  std::string cluster;
};

struct HttpConnectionManagerConfig {
  std::string route_name;
};

using FilterConfig = std::variant<TcpProxyConfig, HttpConnectionManagerConfig>;

// visitor 是一个能处理 variant 中每一种类型的 callable object。
struct ConfigPrinter {
  // 定义 operator() 后，ConfigPrinter 对象就能像函数一样调用：
  // ConfigPrinter printer;
  // printer(tcp_config);
  void operator()(const TcpProxyConfig &tcp) const {
    std::cout << "tcp_proxy cluster=" << tcp.cluster << '\n';
  }

  // 这是同名函数的另一个 overload。compiler 会根据实参类型选择它。
  void operator()(const HttpConnectionManagerConfig &http) const {
    std::cout << "http_connection_manager route=" << http.route_name << '\n';
  }
};

void printConfig(const FilterConfig &config) {
  // ConfigPrinter{} 创建一个临时 visitor 对象。
  // std::visit 查看 config 当前保存的类型，然后调用对应的 operator()：
  //   保存 TcpProxyConfig                -> printer(tcp)
  //   保存 HttpConnectionManagerConfig   -> printer(http)
  std::visit(ConfigPrinter{}, config);
}

} // namespace

int main() {
  const std::vector<FilterConfig> configs{
      TcpProxyConfig{"database"},
      HttpConnectionManagerConfig{"local_route"},
      TcpProxyConfig{"cache"},
  };

  for (const FilterConfig &config : configs) {
    printConfig(config);
  }

  // 练习：给 FilterConfig 增加 UdpProxyConfig，但先不要修改 ConfigPrinter。
  // std::visit 要求 visitor 能处理所有 alternative，compiler 会指出缺少的
  // operator() overload；补齐 overload 后才能恢复编译。
}
