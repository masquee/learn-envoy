# Envoy C++ / Abseil 学习示例

这里的每个程序只讲一个核心概念，并借用 listener、cluster、route、filter 等 Envoy 语境来举例。建议按编号阅读：先猜输出，再运行，最后修改练习。

## 快速开始

要求：支持 C++17 的编译器、CMake 3.20+、Abseil。

macOS（Homebrew）：

```bash
brew install cmake abseil
cmake -S examples/cpp -B /tmp/learn-envoy-cpp-build \
  -DCMAKE_PREFIX_PATH="$(brew --prefix abseil)"
cmake --build /tmp/learn-envoy-cpp-build -j
ctest --test-dir /tmp/learn-envoy-cpp-build --output-on-failure
```

运行单个例子：

```bash
/tmp/learn-envoy-cpp-build/status_or
```

也可以使用与 Envoy 相同的 Bazel 生态。首次执行会下载 Bazel/Abseil：

```bash
cd examples/cpp
bazel run //:status_or
bazel build //...
```

如果想了解 Bazel 如何根据 `bazel_dep` 找到并下载 Abseil，以及 `@abseil-cpp//absl/status:statusor` 这类 label 如何阅读，请参阅 [`BAZEL.md`](BAZEL.md)。

## 学习顺序

| # | 源码 | 主题 | 阅读 Envoy 时关注什么 |
|---|---|---|---|
| 01 | [`01_status_or.cc`](01_status_or.cc) | `absl::Status` / `absl::StatusOr<T>` | 可恢复错误、错误码、错误向上传播 |
| 02 | [`02_string_view.cc`](02_string_view.cc) | `absl::string_view` | non-owning view 与对象 lifetime |
| 03 | [`03_optional.cc`](03_optional.cc) | `absl::optional<T>` | “没有值”和默认值的区别 |
| 04 | [`04_span.cc`](04_span.cc) | `absl::Span<T>` | 不复制地传递一段连续数据 |
| 05 | [`05_flat_hash_map.cc`](05_flat_hash_map.cc) | `absl::flat_hash_map` | 高效 lookup、遍历顺序不稳定 |
| 06 | [`06_strings.cc`](06_strings.cc) | Abseil strings | `StrCat`、`StrSplit`、`StrJoin`、`SimpleAtoi` |
| 07 | [`07_cleanup.cc`](07_cleanup.cc) | `absl::Cleanup` | scope exit cleanup 与 early return |
| 08 | [`08_smart_pointers.cc`](08_smart_pointers.cc) | RAII / smart pointer | ownership、析构、`weak_ptr` |
| 09 | [`09_lambdas_callbacks.cc`](09_lambdas_callbacks.cc) | lambda / callback | asynchronous callback 的 capture lifetime |
| 10 | [`10_variant_basics.cc`](10_variant_basics.cc) | `std::variant` 基础 | 多选一类型、`holds_alternative`、`get`、`get_if` |
| 11 | [`11_variant_visit.cc`](11_variant_visit.cc) | `std::visit` | 普通 visitor struct 和 `operator()` overload |
| 12 | [`12_variant_overloaded.cc`](12_variant_overloaded.cc) | `Overloaded` visitor | 用 variadic template 把多个 lambda 合成 visitor |
| 13 | [`13_inlined_vector.cc`](13_inlined_vector.cc) | `absl::InlinedVector` | 小集合避免 heap allocation |

`std::variant` 部分请严格按照 10 → 11 → 12 阅读。专题说明和图解见 [`VARIANT.md`](VARIANT.md)；第 12 个例子是进阶语法，不理解它不会影响 `variant` 和 `visit` 的正常使用。

## 先重点掌握 `StatusOr`

把 `absl::StatusOr<T>` 理解成“要么是一个 `T`，要么是一个错误”。基本使用模式是：

```cpp
absl::StatusOr<int> result = parsePort(text);
if (!result.ok()) {
  return result.status();
}
int port = *result;
```

几个容易踩的坑：

- 读取值之前先检查 `ok()`；不要用 `value()` 代替错误处理。
- `StatusOr<T*>` 成功时仍可能装着 `nullptr`；成功只代表存在一个 `T*` 值。
- 返回错误时选择准确的 canonical code，例如 `InvalidArgumentError`、`NotFoundError`。
- 在边界处给错误补充语境，否则多层调用后只剩下类似 “not found” 的模糊信息。
- `StatusOr` 适合预期内、可处理的失败；真正违反程序 invariant 的情况通常不应伪装成普通错误。

## 建议练习

1. 给 `01_status_or.cc` 的 `parseEndpoint` 增加空 host 校验，并返回 `InvalidArgumentError`。
2. 修改 `02_string_view.cc`，返回 URL 中 `/path` 之前的 host，观察它为何仍不需要复制。
3. 给 `09_lambdas_callbacks.cc` 连续 `post` 三个 callback，确认执行顺序和 capture 的值。
4. 给 `11_variant_visit.cc` 增加 `UdpProxyConfig`；观察 compiler 如何提示 visitor 缺少对应的 `operator()` overload。

## 阅读方法

看到 Envoy 里的类型时，先问三个问题：谁拥有对象、对象能活多久、失败如何表达。C++ 中大量难点最终都落在 ownership、lifetime 和 error handling 上。示例故意保持独立，不依赖 Envoy 源码；掌握后再对照真实调用链，会比一次追进大型 class hierarchy 更容易。
