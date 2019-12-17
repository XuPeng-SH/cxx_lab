# libfiu 用户空间错误注入

* [主页](https://blitiri.com.ar/p/libfiu/)
* [官方文档](https://blitiri.com.ar/p/libfiu/doc/guide.html)
* [GITHUB 地址](https://github.com/albertito/libfiu)

## 简介
> **C library**
> **错误注入方式**
- 在用户代码内集成libfiu库，使用API注入相关错误, 建议用于自助开发的模块错误注入
- 基于libfiu开发独立动态库，实现目标模块相关接口, 用于第三方模块错误注入, 比如POSIX接口
> **API 分类**
- Core API 注入错误点
- Control API 控制注入的错误点
> **基本思想**
有一段代码需要校验是否有足够的空间存储文件
```c
size_t free_space() {
[code to find out how much free space there is]
return space;
}

bool file_fits(FILE *fd) {
if (free_space() < file_size(fd)) {
    return false;
}
return true;
}
```
很难遇到存储空间不足的情况，这种情况下的测试比较难做。我们可以使用libfiu轻松的模拟出存储空间不足的情况:
```c
size_t free_space() {
fiu_return_on("no_free_space", 0);
[code to find out how much free space there is]
return space;
}

bool file_fits(FILE *fd) {
if (free_space() < file_size(fd)) {
    return false;
}
return true;
}
```
`fiu_return_on` 在代码中注入了一个名字为 `no_free_space` 的错误点。当这个错误点被使用的时候，`free_space` 会返回 `0`。
在测试代码中，我们只要加入以下代码:
```c
fiu_init(0); // 初始化
fiu_enable("no_free_space", 1, NULL, 0); // 打开 no_free_space 错误
assert(file_fits("tmpfile") == false);
```
`fiu_return_on` 属于core API, `fiu_enable` 属于 control API.

## 使用libfiu
> **安装**
Debian 和 Ubuntu 用户可以直接安装
```bash
>>> apt-get install libfiu-dev fiu-utils
```
源码安装
```bash
>>> git clone https://github.com/albertito/libfiu
>>> cd libfiu
>>> make
>>> make PREFIX=/your/prefix install
```

> **错误注入**
**在需要注入错误的文件加入 `#include <fiu-local.h>`**
**使用 core API 注入错误**
```cpp
fiu_return_on("fault_name", -1);
fiu_exit_on("fault_name");
fiu_fail("fault_name");
 ```
错误注入点的名字带有层级关系
```
"db/insert/*" ==> "db/insert/memtable", "db/insert/immutable", ...
"server/grpc/*" ==> "server/grpc/drop_table", "server/grpc/create_table", ...
```
> **使用 control API 控制错误**

`fiu_enable()`: 强制打开错误
`fiu_enable_random()`: 随机打开错误
`fiu_enable_external()`: `External` 函数返回值满足条件时打开错误
脚本控制错误点
```bash
# 启动错误名为 error_point 的注入错误
>>> fiu-ctrl -c 'enable name=$fault_name' $pid
```
> **启动程序**
```bash
# 使用 fiu-run 启动程序
>>> fiu-run -c 'enable name=$fault_name' $executable_file ... $argv

# 使用 fiu-ls 查看运行 fiu-run 的程序
>>> fiu-ls
```
