# TMSL-Simulator



## 编译环境

Win10 pro x64 cmd

gcc version 8.1.0 (x86_64-posix-sjlj-rev0, Built by MinGW-W64 project)



## 编译方法

 根目录下 `make`



## 执行方法

### 1 普通运行

* `TMSLSimu [nel filename]`

e.g.  `TMSLSimu 0.basic.nel` 

### 2 性能测试

* `TMSLSimu [nel filename] P`

e.g.  `TMSLSimu Big_test.nel P`

### 3 Cycle瞬时状态查询

* `TMSLSimu [nel filename] C[cycle number]`

e.g.  `TMSLSimu Big_test.nel C1000`

* **注**：该时钟周期的模拟器状态将被打印到 `printState.csv` 文件中