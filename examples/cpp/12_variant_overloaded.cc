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

using FilterConfig = std::variant<TcpProxyConfig, HttpConnectionManagerConfig>;

// 这是一个进阶 helper：把多个 lambda 合成一个 visitor。
// 建议先完全理解 11_variant_visit.cc 中的 ConfigPrinter 再看这里。
//
// Callables... 是 template parameter pack。例如传入两个 lambda 时，可以把
// Overloaded<Lambda1, Lambda2> 想象成同时继承 Lambda1 和 Lambda2 的 class。
template <class... Callables> struct Overloaded : Callables... {
  // 每个 lambda 内部都有自己的 operator()。
  // 这行使用 pack expansion，把所有 operator() 引入当前 class，组成 overload
  // set。
  using Callables::operator()...;
};

// 这是 C++17 deduction guide。
// 它让 compiler 能从 Overloaded{lambda1, lambda2} 自动推导出上面的 template
// types， 不需要手工写出匿名且无法直接拼写的 lambda 类型。
template <class... Callables>
Overloaded(Callables...) -> Overloaded<Callables...>;

void printConfig(const FilterConfig &config) {
  // 第一个 lambda 相当于 ConfigPrinter 中处理 TCP 的 operator() overload。
  // 第二个 lambda 相当于处理 HTTP 的 operator() overload。
  const auto printer = Overloaded{
      [](const TcpProxyConfig &tcp) {
        std::cout << "tcp_proxy cluster=" << tcp.cluster << '\n';
      },
      [](const HttpConnectionManagerConfig &http) {
        std::cout << "http_connection_manager route=" << http.route_name
                  << '\n';
      },
  };

  // 对 std::visit 来说，printer 与上一个 demo 的 ConfigPrinter{} 作用相同。
  std::visit(printer, config);
}

} // namespace

int main() {
  printConfig(TcpProxyConfig{"database"});
  printConfig(HttpConnectionManagerConfig{"local_route"});
}
