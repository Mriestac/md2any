# 打包说明

## 目标

生成 Windows 发布目录：

```text
dist/md2any
```

目录中包含：

- `md2any.exe`
- Qt DLL
- Qt plugins
- QML imports
- MinGW 运行库

Pandoc 当前不内置，用户机器需要单独安装 Pandoc 并加入 PATH。

## 前置要求

确认以下命令可运行：

```powershell
cmake --version
ninja --version
windeployqt --help
pandoc --version
```

## 打包命令

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\configure.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
```

`package.ps1` 会先清理旧的 `dist/md2any`，再复制最新 `md2any.exe` 并运行 `windeployqt`。

## 打包验证

启动发布版：

```powershell
.\dist\md2any\md2any.exe
```

手动验证：

1. 准备一个 Markdown 文件。
2. 启动 `dist/md2any/md2any.exe`。
3. 选择输入文件。
4. 输出格式选择 `html`。
5. 选择输出路径。
6. 点击“开始转换”。
7. 确认生成 HTML 文件。

## 已知提示

`windeployqt` 可能输出：

```text
Warning: Cannot find any version of the dxcompiler.dll and dxil.dll.
```

当前版本短启动验证通过。若后续引入更多图形或 Shader 特性，需要重新评估该警告。

## 发布前检查

- `ctest` 全部通过
- `dist/md2any/md2any.exe` 能启动
- 打包版能完成 Markdown 到 HTML 转换
- Pandoc 不存在时有中文错误提示
- PDF 转换失败时能显示 Pandoc 错误输出
