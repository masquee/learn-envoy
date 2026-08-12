# `std::variant` 与 `std::visit`

这部分最重要的不是模板语法，而是先建立一个简单模型：

```text
std::variant<A, B> value

value 当前保存 A  ──┐
                    ├── 同一时间只会有一个 active value
value 当前保存 B  ──┘
```

建议依次运行：

```bash
cd examples/cpp
bazel run //:variant_basics
bazel run //:variant_visit
bazel run //:variant_overloaded
```

## 1. `variant` 解决什么问题

假设一个 filter config 可能是 TCP，也可能是 HTTP：

```cpp
struct TcpProxyConfig {
  std::string cluster;
};

struct HttpConnectionManagerConfig {
  std::string route_name;
};

using FilterConfig =
    std::variant<TcpProxyConfig, HttpConnectionManagerConfig>;
```

`FilterConfig` 不是同时包含两个配置。每一个 `FilterConfig` 对象只保存其中一种：

```cpp
FilterConfig tcp = TcpProxyConfig{"database"};
FilterConfig http = HttpConnectionManagerConfig{"local_route"};
```

与无类型的 `void*` 或传统 C union 相比，`variant` 记得当前值的真实类型，并负责正确构造和析构对象。

## 2. 第一阶段：直接判断和取值

对应示例：[`10_variant_basics.cc`](10_variant_basics.cc)。

常用 API 可以先记住这四个：

| API | 含义 | 类型不匹配时 |
|---|---|---|
| `config.index()` | 当前类型在 template type list 中的位置 | 不涉及匹配 |
| `holds_alternative<T>(config)` | 当前是否保存 `T` | 返回 `false` |
| `get<T>(config)` | 取得内部的 `T` | 抛 `bad_variant_access` |
| `get_if<T>(&config)` | 尝试取得指向内部 `T` 的 pointer | 返回 `nullptr` |

最直观的模式是：

```cpp
if (std::holds_alternative<TcpProxyConfig>(config)) {
  const TcpProxyConfig& tcp = std::get<TcpProxyConfig>(config);
  // 使用 tcp
}
```

更紧凑且不会因类型不匹配而抛异常的是：

```cpp
if (const auto* tcp = std::get_if<TcpProxyConfig>(&config)) {
  // tcp 是 pointer，因此使用 tcp->cluster。
}
```

`get_if` 返回的 pointer 指向 `variant` 内部，并不拥有对象。不要让该 pointer 活得比 `variant` 更久；给 `variant` 赋另一种类型的值后，之前取得的 pointer/reference 也会失效。

## 3. 为什么需要 `std::visit`

只有两种类型时，可以一直写 `holds_alternative + get`。类型增多后会形成重复的判断链：

```cpp
if (holds A) {
  handle A;
} else if (holds B) {
  handle B;
} else if (holds C) {
  handle C;
}
```

`std::visit` 把“识别当前类型”的工作交给标准库：

```text
variant 当前保存 A → visitor(A)
variant 当前保存 B → visitor(B)
```

它的基本形式是：

```cpp
std::visit(visitor, variant_value);
```

其中 `visitor` 必须是一个 callable，并且能接收 `variant` 中的每一种类型。

## 4. 第二阶段：普通 visitor struct

对应示例：[`11_variant_visit.cc`](11_variant_visit.cc)。

最容易读懂的 visitor 是普通 struct：

```cpp
struct ConfigPrinter {
  void operator()(const TcpProxyConfig& tcp) const {
    // 处理 TCP
  }

  void operator()(const HttpConnectionManagerConfig& http) const {
    // 处理 HTTP
  }
};
```

`operator()` 让对象可以像函数一样被调用：

```cpp
ConfigPrinter printer;
printer(TcpProxyConfig{"database"});
```

上一行等价于显式写：

```cpp
printer.operator()(TcpProxyConfig{"database"});
```

两个 `operator()` 是 function overload。compiler 会根据实参类型选择正确版本。因此：

```cpp
std::visit(ConfigPrinter{}, config);
```

可以理解为标准库在内部做了：

```text
如果 config 保存 TCP  → ConfigPrinter{}(tcp_config)
如果 config 保存 HTTP → ConfigPrinter{}(http_config)
```

当给 `FilterConfig` 增加第三种类型时，visitor 也必须提供处理第三种类型的 overload。漏掉时会产生 compile error，这正是类型安全带来的好处：新增类型后，compiler 帮助找出尚未处理的位置。

## 5. 第三阶段：`Overloaded` 与 lambda

对应示例：[`12_variant_overloaded.cc`](12_variant_overloaded.cc)。这是进阶写法，可以最后再学。

普通 visitor 需要先声明一个有名字的 struct。有时处理逻辑很短，代码会使用多个 lambda：

```cpp
const auto printer = Overloaded{
    [](const TcpProxyConfig& tcp) {
      // 处理 TCP
    },
    [](const HttpConnectionManagerConfig& http) {
      // 处理 HTTP
    },
};
```

每个 lambda 本质上都是一个带 `operator()` 的匿名 class。`Overloaded` 同时继承这些匿名 class，并把它们的 `operator()` 放进同一个 overload set：

```cpp
template <class... Callables>
struct Overloaded : Callables... {
  using Callables::operator()...;
};
```

所以 `Overloaded{lambda1, lambda2}` 与前面的 `ConfigPrinter` 在用途上相同，只是写法更紧凑。

如果下面这些语法还陌生，可以先跳过：

- variadic template：`class... Callables`；
- parameter pack expansion：`Callables...` 和 `operator()...`；
- inheritance；
- class template argument deduction guide。

它们是实现 helper 所需的知识，不是使用 `std::variant` 的前置条件。

## 6. 什么时候适合使用 `variant`

适合：

- 候选类型集合是有限且编译时已知的；
- 类型之间没有自然、稳定的共同 base class；
- 希望使用 value semantics；
- 希望新增候选类型后由 compiler 检查 visitor 是否完整。

不一定适合：

- 只有“有值/没有值”两种状态：优先考虑 `optional<T>`；
- 只有“成功值/错误”两种状态：优先考虑 `StatusOr<T>`；
- 类型集合需要由外部插件持续扩展：polymorphic base class 可能更自然；
- 所有类型本来就共享稳定 interface：virtual dispatch 可能更容易理解。

## 7. 常见错误

### 未检查类型就调用 `get`

```cpp
std::get<TcpProxyConfig>(config);
```

如果当前不是 TCP，会抛出 `std::bad_variant_access`。在通常关闭 exceptions 的 Envoy 风格代码中，应优先使用 `get_if` 或完整的 visitor。

### 保存内部对象的 pointer/reference 太久

```cpp
const auto* tcp = std::get_if<TcpProxyConfig>(&config);
config = HttpConnectionManagerConfig{"route"};
// tcp 已失效，不能再解引用。
```

### visitor 漏掉一种类型

这通常会表现为很长的 template compile error。先忽略标准库内部报错，检查 visitor 是否能接收 `variant<...>` 列出的每一种类型。

### visitor 的返回类型不一致

如果需要从 `std::visit` 返回值，各 overload 应返回相同类型。例如不能让 TCP 分支返回 `std::string`，HTTP 分支却返回 `int`。

## 8. 最小记忆模型

先记住以下代码就够了：

```cpp
using Value = std::variant<A, B>;

struct Visitor {
  void operator()(const A& a) const { /* handle A */ }
  void operator()(const B& b) const { /* handle B */ }
};

Value value = A{};
std::visit(Visitor{}, value);
```

它表达的就是：

```text
Value 是 A 或 B；std::visit 根据当前类型调用对应处理函数。
```
