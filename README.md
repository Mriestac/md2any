# md2any

md2any 是一个基于 Pandoc 的 Windows 桌面文档转换工具。界面使用 Qt/QML 构建，后端使用 C++ 调用 Pandoc，目标是让用户不用手写命令也能完成 Markdown 到 HTML、DOCX、PDF 的转换。

## 功能

- 中文图形界面
- 选择 Markdown 输入文件
- 选择输出文件路径
- 支持输出 `html`、`docx`、`pdf`
- 自动修正输出扩展名
- 检查输入路径、输出目录、覆盖策略和 Pandoc 状态
- 使用 Pandoc 执行真实转换
- HTML 输出默认启用 `--standalone --mathjax`，便于显示 TeX 数学公式
- 转换中显示状态，可取消转换
- 显示 stdout、stderr 和错误信息
- 保存最近输入目录、输出目录和输出格式
- 打包为 Windows 可执行程序目录

## 环境要求

- Windows
- Qt 6，当前验证版本为 Qt 6.11.0 MinGW 64-bit
- CMake，当前验证版本为 3.30.5
- Ninja，当前验证版本为 1.12.1
- Pandoc，当前验证版本为 3.9.0.2

Pandoc 不随 md2any 打包，需要用户单独安装并加入 PATH。

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
- fake Pandoc 成功和失败执行
- 设置读取和保存
- AppController 初始状态、路径规范化、转换成功/失败状态

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

请确认 `pandoc --version` 在 PowerShell 中可运行。Pandoc 默认需要加入系统 PATH。

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

- 增加打开输出文件
- 增加 Pandoc 路径设置
- 增加转换历史
- 增加批量转换
- 增加 PDF 引擎配置
- 增加更完整的 UI 自动化测试
