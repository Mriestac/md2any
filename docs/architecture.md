# md2any 架构文档

## 项目状态

当前仓库基本为空，仅包含 `.git` 和一个编码异常的 `README.md`。本项目按新项目初始化处理，不需要兼容既有源码结构。

## 技术栈

### 选择

- 语言：C++17
- UI 框架：Qt 6
- UI 描述：QML / Qt Quick Controls 2
- 构建系统：CMake
- Pandoc 调用：Qt `QProcess`
- 配置持久化：`QSettings`
- 测试框架：Qt Test
- 打包工具：CMake + `windeployqt`

### 理由

- Qt/C++ 原生支持 QML，符合 UI 用 QML 设计的要求
- C++ 后端与 QML 集成直接，适合长期维护桌面应用
- `QProcess` 能可靠调用 Pandoc 并捕获 stdout、stderr 和退出码
- `QSettings` 适合保存轻量桌面应用配置，不需要数据库
- CMake 是 Qt C++ 项目的标准构建方式
- `windeployqt` 是 Windows 分发 Qt 应用的常规方案

## 目录结构

```text
F:\code\md2any
├─ README.md
├─ CMakeLists.txt
├─ docs
│  ├─ requirements.md
│  ├─ ui-design.md
│  ├─ architecture.md
│  ├─ tasks.md
│  ├─ test-plan.md
│  └─ packaging.md
├─ src
│  ├─ main.cpp
│  ├─ app_controller.h
│  ├─ app_controller.cpp
│  ├─ conversion_task.h
│  ├─ conversion_result.h
│  ├─ pandoc_runner.h
│  ├─ pandoc_runner.cpp
│  ├─ settings_store.h
│  ├─ settings_store.cpp
│  └─ qml
│     └─ Main.qml
├─ tests
│  ├─ CMakeLists.txt
│  ├─ test_pandoc_runner.cpp
│  └─ test_settings_store.cpp
├─ scripts
│  ├─ configure.ps1
│  ├─ build.ps1
│  ├─ test.ps1
│  └─ package.ps1
└─ packaging
   └─ README.md
```

## 模块职责

### `main.cpp`

- 创建 `QGuiApplication`
- 创建 `QQmlApplicationEngine`
- 初始化后端对象
- 把后端对象暴露给 QML
- 加载 `Main.qml`
- 处理开发环境和打包环境下的 QML 资源路径

### `app_controller.h/.cpp`

- 作为 QML 与 C++ 后端的桥接层
- 使用 `QObject`、`Q_PROPERTY`、`Q_INVOKABLE` 暴露状态和操作
- 把 UI 输入转换为转换任务
- 调用 `PandocRunner`
- 向 QML 发出状态、日志和结果信号

### `conversion_task.h`

- 定义转换任务字段
- 包含输入路径、输出路径、输出格式、覆盖选项和额外参数

### `conversion_result.h`

- 定义转换结果字段
- 包含成功状态、命令、退出码、stdout、stderr、输出路径和用户消息

### `pandoc_runner.h/.cpp`

- 检测 Pandoc 是否可用
- 校验输入路径、输出路径和格式
- 构建 Pandoc 参数列表
- 使用 `QProcess` 执行 Pandoc
- 返回统一转换结果
- 不通过 shell 拼接命令

### `settings_store.h/.cpp`

- 使用 `QSettings` 读取应用配置
- 保存 Pandoc 路径、最近目录和默认输出格式
- 配置不存在时返回默认值
- 配置异常时不影响主流程

### `src/qml/Main.qml`

- 实现主窗口界面
- 文件选择、格式选择、覆盖选项、转换按钮、状态和日志区域
- 不直接拼接 Pandoc 命令
- 只调用 C++ 后端公开方法

## 数据模型

### ConversionTask

- `inputPath: QString`
- `outputPath: QString`
- `outputFormat: QString`
- `overwrite: bool`
- `extraArgs: QStringList`

### ConversionResult

- `success: bool`
- `command: QStringList`
- `exitCode: int`
- `standardOutput: QString`
- `standardError: QString`
- `outputPath: QString`
- `message: QString`

### AppSettings

- `pandocPath: QString`
- `lastInputDir: QString`
- `lastOutputDir: QString`
- `lastOutputFormat: QString`

## 状态管理方案

MVP 使用简单状态，不引入复杂状态库。

- QML 管理当前表单输入
- C++ `AppController` 管理转换状态
- `AppController` 通过属性和信号通知 QML 更新
- 转换期间 UI 禁用相关控件
- 转换完成后恢复 UI

状态值：

- `idle`
- `checking`
- `running`
- `success`
- `failed`

## 文件读写和配置方案

- MVP 不使用数据库
- 用户文档只通过 Pandoc 读取输入文件并写入输出文件
- 应用配置使用 `QSettings`
- 测试中应允许注入临时配置作用域或隔离配置组织名

## Pandoc 调用方案

命令构建示例：

```text
pandoc input.md -o output.html
```

约束：

- 使用 `QProcess::setProgram` 和 `QProcess::setArguments`
- 不通过 `cmd.exe` 或 shell 拼接字符串
- 捕获 stdout 和 stderr
- 设置合理超时时间
- 对路径存在性和输出目录提前校验

## 构建、运行、打包方式

### 配置

```powershell
cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

### 构建

```powershell
cmake --build build-ninja
```

### 测试

```powershell
ctest --test-dir build-ninja --output-on-failure
```

### 运行

```powershell
.\build-ninja\md2any.exe
```

具体路径会根据生成器不同变化，脚本中应封装常用命令。

### 打包

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

打包时必须验证：

- QML 文件或 Qt Resource 资源被包含
- Qt Quick Controls 运行库可加载
- 打包后 exe 能启动
- Pandoc 查找逻辑在打包环境中仍然可用

## 风险和约束

- Pandoc 需要用户本机安装，MVP 不内置 Pandoc
- PDF 转换可能依赖 LaTeX 或其他 PDF 引擎
- QML 资源路径在开发环境和打包环境不同，需要统一处理，优先考虑 Qt Resource
- Windows 中文路径、空格路径必须测试
- UI 不应直接拼接命令，避免维护困难和命令注入风险
- C++/Qt 环境需要用户本机安装 Qt 6 和 CMake
