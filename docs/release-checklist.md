# 最终验收清单

## 当前验证结果

- 验证日期：2026-05-08
- 构建目录：`build-ninja`
- 发布目录：`dist/md2any`
- 自动化测试：`5/5 passed`
- 打包：已通过
- 打包版短启动：已通过
- 打包版 GUI 手动 Markdown 到 HTML：已通过
- 已知打包提示：`windeployqt` 提示缺少 `dxcompiler.dll` / `dxil.dll`，当前短启动不受影响

## 构建

- [x] `scripts/configure.ps1` 可执行
- [x] `scripts/build.ps1` 可执行
- [x] 生成 `build-ninja/md2any.exe`

## 测试

- [x] `scripts/test.ps1` 全部通过
- [x] 模型测试通过
- [x] PandocRunner 测试通过
- [x] SettingsStore 测试通过
- [x] AppController 测试通过

## 功能

- [x] 应用可启动
- [x] 界面为中文
- [x] 可选择 Markdown 输入文件
- [x] 可选择输出路径
- [x] 可选择 `html`、`docx`、`pdf`
- [x] 输出路径无扩展名时会自动补齐
- [x] 可检查配置
- [x] 可执行真实 Pandoc 转换
- [x] 转换中可取消
- [x] 转换成功后显示输出路径
- [x] 转换失败后显示错误信息
- [x] 最近目录和输出格式可保存
- [x] 打包版 GUI 手动 Markdown 到 HTML 转换已通过

## 打包

- [x] `scripts/package.ps1` 可执行
- [x] 生成 `dist/md2any/md2any.exe`
- [x] 打包版可启动
- [x] 打包版可完成 Markdown 到 HTML 转换

## 错误处理

- [x] 未选择输入文件时有中文提示
- [x] 输入文件不存在时有中文提示
- [x] 输出目录不存在时有中文提示
- [x] 输出文件已存在且未允许覆盖时有中文提示
- [x] Pandoc 不存在时有中文提示
- [x] Pandoc 转换失败时显示 stderr
