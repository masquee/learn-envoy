# Bazel 基础：依赖是从哪里获取的

本目录使用 Bazel 的 Bzlmod 管理第三方依赖。以 `absl::StatusOr` 示例依赖的 Abseil 为例，完整链路是：

```text
MODULE.bazel 声明模块名和版本
        ↓
Bazel 查询 module registry
        ↓
registry 提供源码 URL、checksum 等元数据
        ↓
Bazel 下载并创建外部仓库 @abseil-cpp
        ↓
BUILD.bazel 引用外部仓库中的 target
```

## 1. `MODULE.bazel` 声明外部模块

[`MODULE.bazel`](MODULE.bazel) 中有：

```starlark
module(
    name = "learn_envoy_cpp_examples",
    version = "0.1.0",
)

bazel_dep(name = "abseil-cpp", version = "20260107.1")
bazel_dep(name = "rules_cc", version = "0.2.22")
```

`module(...)` 描述当前项目；`bazel_dep(...)` 声明当前项目需要的外部模块。

这里没有写 Abseil 的 Git URL。Bazel 默认从 Bazel Central Registry（BCR）查找 `abseil-cpp@20260107.1`，registry 再通过该版本的 `source.json` 告诉 Bazel：

- 源码 archive 的下载地址；
- 校验源码的 integrity/checksum；
- 解压时使用的 `strip_prefix`；
- 必要时需要应用的 patches。

默认 registry 是：

```text
https://bcr.bazel.build/
```

这个版本的关键 registry 文件位于：

```text
https://bcr.bazel.build/modules/abseil-cpp/20260107.1/MODULE.bazel
https://bcr.bazel.build/modules/abseil-cpp/20260107.1/source.json
```

因此，这里的依赖映射过程是：

```text
abseil-cpp + 20260107.1
            ↓
Bazel Central Registry
            ↓
Abseil source archive
```

公司内部也可以通过 Bazel 的 `--registry` 配置使用私有 registry，或者设置多个 registry。registry 负责模块发现，真正的源码不一定存储在 registry 自身。

## 2. `BUILD.bazel` 使用模块中的 target

模块下载并解析后，[`BUILD.bazel`](BUILD.bazel) 可以引用它提供的 target：

```starlark
cc_binary(
    name = "status_or",
    srcs = ["01_status_or.cc"],
    deps = [
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
    ],
)
```

Bazel label 的通用格式是：

```text
@repository//package/path:target
```

所以：

```text
@abseil-cpp//absl/status:statusor
└────┬─────┘ └─────┬─────┘ └──┬───┘
  外部仓库       package       target
```

- `@abseil-cpp`：`bazel_dep` 引入的外部仓库；
- `//absl/status`：Abseil 仓库中的 Bazel package；
- `:statusor`：这个 package 的 `BUILD.bazel` 所定义的 target。

代码中的 C++ `#include` 与 Bazel target 是两个相关但不同的概念：

```cpp
#include "absl/status/statusor.h"
```

```starlark
deps = ["@abseil-cpp//absl/status:statusor"]
```

前者让 compiler 看见 header 中的声明，后者让 Bazel 知道 header、编译选项和 link dependencies 从哪里来。只写 `#include` 而漏掉 `deps`，通常会得到 sandbox 内找不到 header，或者依赖检查失败之类的错误。

## 3. `MODULE.bazel.lock` 锁定解析结果

第一次解析依赖后，Bazel 会生成 [`MODULE.bazel.lock`](MODULE.bazel.lock)。里面会记录：

- 使用过的 registry 元数据及其 hash；
- 最终选择的模块版本；
- module extensions 的解析结果；
- 与当前平台相关的部分依赖信息。

它与其他工具中的 `package-lock.json`、`Cargo.lock` 作用相近：`MODULE.bazel` 表达依赖要求，lock file 记录一次可复现的解析结果。

一般应将 `MODULE.bazel.lock` 提交到 Git。修改 `MODULE.bazel` 后执行 Bazel，lock file 可能随之更新；应该一起 review，避免在不知情的情况下改变依赖树。

## 4. 查看依赖关系

在本目录运行：

```bash
cd examples/cpp
bazel mod graph
```

它会输出 module dependency graph，可以用来回答：

- 当前项目直接依赖哪些模块；
- Abseil 又引入了哪些 transitive dependencies；
- 多个版本要求最终选择了哪个版本。

构建一个 target：

```bash
bazel build //:status_or
```

运行一个 target：

```bash
bazel run //:status_or
```

构建当前 module 下的全部 target：

```bash
bazel build //...
```

这里的本仓库 label 也遵循同一规则：

```text
//:status_or
│  └── target
└───── 当前 workspace 根 package
```

省略 `@repository` 表示引用当前 repository；`//...` 表示递归匹配当前 repository 中所有 package 的 target。

## 5. 下载后的依赖放在哪里

Bazel 不会把 Abseil 源码复制进本仓库。它会放到 Bazel output base 管理的外部依赖区域，并通过 sandbox/cache 参与构建。

可以查看当前 output base：

```bash
bazel info output_base
```

通常不应手工修改其中的文件，因为它们是 Bazel 管理的缓存。需要升级或修补依赖时，应修改 `MODULE.bazel`、registry 配置或使用 Bzlmod 提供的 override 机制。

## 6. 本例中的两个外部模块

本项目直接声明了两个模块：

| module | 用途 |
|---|---|
| `abseil-cpp` | 提供 `StatusOr`、`string_view`、`Span`、containers 等 C++ 库 |
| `rules_cc` | 提供 Bazel 9 使用的 `cc_binary` 等 C++ build rules |

因此 [`BUILD.bazel`](BUILD.bazel) 开头需要显式加载 `cc_binary`：

```starlark
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
```

这也是一个外部 target 引用，只不过它通过 `load` 加载的是 Starlark rule，而不是通过 `deps` 链接 C++ library。

## 7. 阅读 Envoy 的 Bazel 文件时

看到一个 dependency label，可以按以下顺序拆解：

1. 是否以 `@` 开头？是则来自外部 repository，否则来自 Envoy 自己的 repository。
2. `//` 后面是哪一个 package？去对应目录寻找 `BUILD` 或 `BUILD.bazel`。
3. `:` 后面是哪一个 target？在 BUILD 文件中寻找其定义。
4. 外部 repository 从哪来？优先查看 `MODULE.bazel`、module extensions 和 lock file；较旧项目还可能使用 `WORKSPACE`。
5. 它是直接依赖还是 transitive dependency？用 `bazel mod graph` 和 Bazel query 工具继续追踪。

最重要的区分是：

```text
MODULE.bazel：获得哪些外部仓库
BUILD.bazel：怎样把源码组织成 target，以及 target 依赖谁
MODULE.bazel.lock：本次外部依赖解析出了什么结果
```
