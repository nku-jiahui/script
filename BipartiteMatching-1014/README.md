# 图匹配

用于处理 ACC 与 Func 的变量匹配问题：使用匈牙利算法，先求出所有变量间的最大匹配，然后扫描所有的 ACC 与 Func 对，找到所有满足了的。

## 构建

直接编译链接“src”目录下的所有“.cc”文件即可。

```bash
$ g++ src/AccGraph.cc src/Hungary.cc src/H2O.cc -O2 -Wall -o build/bimatch
```

## 运行

直接启动程序，以输入的文本文件为命令行参数：

```bash
$ build/bimatch test/BiGraph.txt
```

