# md2any 任务拆解

## 1. 项目初始化或现有项目梳理

### T1.1 梳理现状

- 任务目标：确认仓库状态和现有文件
- 涉及文件：`README.md`
- 实现步骤：读取目录、读取 README、检查 git 状态
- 验收方式：输出项目状态总结
- 风险点：README 当前编码异常，需要后续重写为 UTF-8

### T1.2 初始化 Qt/C++ 项目结构

- 任务目标：创建 CMake、源码、QML、测试和脚本目录
- 涉及文件：`CMakeLists.txt`、`src/main.cpp`、`src/qml/Main.qml`、`tests/CMakeLists.txt`
- 实现步骤：添加 Qt 6 CMake 配置、启用 C++17、创建最小目标
- 验收方式：`cmake -S . -B build` 配置成功
- 风险点：本机 Qt 6 和 CMake 版本需要匹配

### T1.3 创建最小应用入口

- 任务目标：能启动一个最小 QML 窗口
- 涉及文件：`src/main.cpp`、`src/qml/Main.qml`
- 实现步骤：创建 `QGuiApplication`、`QQmlApplicationEngine`、加载 QML
- 验收方式：构建后启动 `md2any.exe` 显示窗口
- 风险点：QML 模块路径和 Qt Quick Controls 依赖可能缺失

## 2. 数据模型和基础结构

### T2.1 定义转换数据模型

- 任务目标：统一转换任务和结果字段
- 涉及文件：`src/conversion_task.h`、`src/conversion_result.h`
- 实现步骤：定义结构体、支持格式常量、基础校验辅助函数
- 验收方式：模型相关测试通过
- 风险点：字段命名需与需求文档保持一致

### T2.2 定义配置模型

- 任务目标：定义应用配置默认值
- 涉及文件：`src/settings_store.h`、`src/settings_store.cpp`
- 实现步骤：使用 `QSettings` 封装默认 Pandoc 路径、最近目录和默认输出格式
- 验收方式：配置测试通过
- 风险点：测试不要污染真实用户配置

## 3. 核心业务逻辑

### T3.1 实现 Pandoc 检测

- 任务目标：检测 Pandoc 是否可运行
- 涉及文件：`src/pandoc_runner.h`、`src/pandoc_runner.cpp`、`tests/test_pandoc_runner.cpp`
- 实现步骤：使用 `QProcess` 调用 `pandoc --version`，捕获找不到命令和执行失败
- 验收方式：使用 fake program 或可注入执行器测试通过
- 风险点：测试不能依赖本机一定安装 Pandoc

### T3.2 实现参数校验

- 任务目标：转换前发现常见输入错误
- 涉及文件：`src/pandoc_runner.cpp`、`tests/test_pandoc_runner.cpp`
- 实现步骤：校验输入文件、输出路径、输出目录、覆盖策略、输出格式
- 验收方式：异常场景测试通过
- 风险点：Windows 路径和中文路径需正确处理

### T3.3 实现 Pandoc 转换

- 任务目标：真实调用 Pandoc 完成转换
- 涉及文件：`src/pandoc_runner.cpp`
- 实现步骤：构建 `QStringList` 参数、调用 `QProcess`、返回 `ConversionResult`
- 验收方式：成功和失败测试通过；有 Pandoc 环境时可手动转换
- 风险点：PDF 转换可能因外部环境失败

## 4. UI 页面

### T4.1 实现主窗口布局

- 任务目标：完成 MVP 单窗口静态 UI
- 涉及文件：`src/qml/Main.qml`
- 实现步骤：添加文件输入、输出路径、格式选择、覆盖选项、按钮、状态和日志区
- 验收方式：窗口可启动，控件可见且不重叠
- 风险点：QML 控件尺寸需要兼容最小窗口

### T4.2 实现 QML 后端桥接

- 任务目标：让 QML 可以调用 C++ 转换逻辑
- 涉及文件：`src/app_controller.h`、`src/app_controller.cpp`、`src/main.cpp`、`src/qml/Main.qml`
- 实现步骤：暴露 `QObject`、`Q_PROPERTY`、`Q_INVOKABLE` 和信号，QML 调用并显示返回结果
- 验收方式：点击转换按钮能触发后端方法
- 风险点：转换执行必须避免阻塞 UI，当前通过 `QProcess` 异步信号处理

### T4.3 接入文件选择器

- 任务目标：支持选择输入文件和输出路径
- 涉及文件：`src/qml/Main.qml`
- 实现步骤：使用 QML `FileDialog`，更新输入框
- 验收方式：可从 UI 选择路径
- 风险点：QML 文件 URL 与 Windows 路径转换要处理

## 5. 数据持久化

### T5.1 实现配置读写

- 任务目标：保存最近路径和默认格式
- 涉及文件：`src/settings_store.h`、`src/settings_store.cpp`、`tests/test_settings_store.cpp`
- 实现步骤：实现 `QSettings` load/save，支持默认值
- 验收方式：配置测试通过
- 风险点：不要在测试中污染真实用户配置

### T5.2 UI 接入配置

- 任务目标：启动时恢复默认输出格式和最近目录
- 涉及文件：`src/app_controller.cpp`、`src/qml/Main.qml`
- 实现步骤：后端提供配置读取属性，转换成功后保存最近值
- 验收方式：重启后默认格式可恢复
- 风险点：配置失败不应影响主功能

## 6. 搜索、筛选、导出等增强功能

### T6.1 打开输出目录

- 任务目标：转换成功后便捷打开输出目录
- 涉及文件：`src/app_controller.cpp`、`src/qml/Main.qml`
- 实现步骤：使用 `QDesktopServices::openUrl` 打开目录，UI 增加按钮
- 验收方式：转换成功后可打开目录
- 风险点：打开系统程序可能受系统策略限制

### T6.2 转换历史

- 任务目标：记录最近转换任务
- 涉及文件：待定
- 实现步骤：设计历史数据结构、保存成功和失败记录、UI 展示
- 验收方式：可查看最近任务
- 风险点：非 MVP，避免过早实现

## 7. 错误处理和边界情况

### T7.1 统一错误消息

- 任务目标：错误信息对用户可理解
- 涉及文件：`src/pandoc_runner.cpp`、`src/app_controller.cpp`
- 实现步骤：将错误转换为统一 message 和日志
- 验收方式：每类错误都有清晰提示
- 风险点：不要吞掉 Pandoc stderr

### T7.2 Windows 路径边界测试

- 任务目标：覆盖中文路径、空格路径、输出已存在
- 涉及文件：`tests/test_pandoc_runner.cpp`
- 实现步骤：使用 `QTemporaryDir` 构造路径并测试校验逻辑
- 验收方式：测试通过
- 风险点：不同文件系统权限表现不同

## 8. 测试

### T8.1 首批单元测试

- 任务目标：覆盖配置和 Pandoc runner
- 涉及文件：`tests/test_pandoc_runner.cpp`、`tests/test_settings_store.cpp`
- 实现步骤：使用 Qt Test、`QTemporaryDir` 和可注入执行器测试核心逻辑
- 验收方式：`ctest --test-dir build --output-on-failure -C Debug` 全部通过
- 风险点：不要让测试依赖真实 Pandoc

### T8.2 UI 冒烟验证

- 任务目标：确认 QML 可加载
- 涉及文件：`src/main.cpp`、`src/qml/Main.qml`
- 实现步骤：手动启动应用，检查 QML 加载日志
- 验收方式：窗口能打开，无 QML 加载错误
- 风险点：CI 或无桌面环境不一定适合跑 GUI 测试

## 9. 打包和发布

### T9.1 编写打包脚本

- 任务目标：使用 `windeployqt` 生成 Windows 发布目录
- 涉及文件：`scripts/package.ps1`、`docs/packaging.md`
- 实现步骤：构建应用，运行 `scripts/package.ps1` 调用 `windeployqt --qmldir src/qml`
- 验收方式：生成可启动发布目录
- 风险点：QML 资源漏部署会导致 exe 启动失败

### T9.2 打包后验证

- 任务目标：确认发布产物可用
- 涉及文件：`dist/` 或 `build/package/`
- 实现步骤：启动 exe，执行一次 HTML 转换，验证错误提示
- 验收方式：打包版本能完成核心流程
- 风险点：Pandoc 是否内置需要明确，MVP 默认不内置

## 10. 文档完善

### T10.1 更新 README

- 任务目标：提供项目说明、运行、测试、打包文档
- 涉及文件：`README.md`
- 实现步骤：重写编码异常内容，补充命令和目录结构
- 验收方式：README 可指导新用户运行项目
- 风险点：命令必须与实际项目一致

### T10.2 编写打包说明

- 任务目标：说明如何打包和验证发布产物
- 涉及文件：`docs/packaging.md`
- 实现步骤：记录 Qt、CMake、windeployqt、Pandoc 依赖、命令、常见问题和验收清单
- 验收方式：按文档能执行打包流程
- 风险点：Qt 安装路径和生成器差异可能影响命令
