# md2any

md2any 是一个基于 Pandoc 的 Windows 桌面文档转换工具。界面使用 Qt/QML 构建，后端使用 C++ 调用 Pandoc。当前 MVP 支持 Markdown 到 HTML、DOCX、PDF 的转换，后续目标是逐步覆盖 Pandoc 支持的输入和输出格式互转。

## 功能

- 中文图形界面
- 选择输入文件
- 选择 Pandoc 输入格式，默认 `markdown`
- 选择输出文件路径
- 从 Pandoc 动态读取可用输出格式
- 设置、检测并保存 Pandoc 可执行文件路径
- 自动修正输出扩展名
- 按输出格式使用推荐扩展名，例如 `latex` 使用 `.tex`
- 检查输入路径、输出目录、覆盖策略和 Pandoc 状态
- 使用 Pandoc 执行真实转换
- 读取 Pandoc 支持的输入/输出格式，为后续多格式互转打基础
- 显示格式列表来源，Pandoc 不可用时使用保底格式
- 常用输入/输出格式优先显示，其他格式按字母排序
- HTML 输出默认启用 `--standalone --mathjax`，便于显示 TeX 数学公式
- 转换中显示状态，可取消转换
- 转换成功后打开输出文件或输出目录
- 显示 stdout、stderr 和错误信息
- 格式组合转换失败时提示查看 Pandoc 错误输出
- 保存最近输入目录、输出目录、输入格式和输出格式
- 打包为 Windows 可执行程序目录

## 环境要求

- Windows
- Qt 6，当前验证版本为 Qt 6.11.0 MinGW 64-bit
- CMake，当前验证版本为 3.30.5
- Ninja，当前验证版本为 1.12.1
- Pandoc，当前验证版本为 3.9.0.2

Pandoc 不随 md2any 打包，需要用户单独安装。可以加入系统 PATH，也可以在软件界面中填写 `pandoc.exe` 的完整路径并保存。

确保这些命令在 PowerShell 中可用：

```powershell
cmake --version
ninja --version
qmake --version
windeployqt --help
pandoc --version
```

## 本地构建

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

运行：

```powershell
.\build-ninja\md2any.exe
```

## 测试

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1
```

当前测试覆盖：

- 数据模型默认值
- 输出格式校验
- Pandoc 参数校验和命令构建
- 异步 Pandoc 执行流程
- fake Pandoc 成功和失败执行
- Pandoc 输入/输出格式发现
- 输入格式 `-f` 和输出格式 `-t` 命令链路
- 设置读取和保存
- AppController 初始状态、路径规范化、Pandoc 路径设置、转换成功/失败状态

## 打包

首次打包前先配置并构建，再运行打包：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

产物目录：

```text
dist\md2any
```

运行打包版：

```powershell
.\dist\md2any\md2any.exe
```

## 目录结构

```text
docs/               项目文档
scripts/            构建、测试、打包脚本
src/                应用源码
src/qml/            QML 界面
tests/              Qt Test 自动化测试
build-ninja/        Ninja 构建目录
dist/md2any/        打包输出目录
```

## 常见问题

### 找不到 Pandoc

请确认 `pandoc --version` 在 PowerShell 中可运行，或在界面中的 “Pandoc 路径” 填写 `pandoc.exe` 的完整路径，然后点击“检测”和“保存”。

### PDF 转换失败

Pandoc 转 PDF 通常还需要额外 PDF 引擎，例如 LaTeX、Typst 或其他 Pandoc 支持的 PDF 工具。md2any 会显示 Pandoc 返回的错误信息。

### PowerShell 禁止运行脚本

使用：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

### 打包时出现 dxcompiler.dll / dxil.dll 警告

当前应用短启动验证通过。该警告来自 Qt 图形相关依赖扫描，后续如果使用更多 Shader 或图形特性，需要进一步确认 Qt 安装中对应 DLL 是否可用。

## 后续开发建议

- 动态读取 Pandoc 支持的输入格式和输出格式
- 增加常用格式说明和格式组合失败提示优化
- 增加转换历史
- 增加批量转换
- 增加 PDF 引擎配置
- 增加更完整的 UI 自动化测试
