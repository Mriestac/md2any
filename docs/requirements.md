# md2any 需求文档

## 项目目标

md2any 是一个基于 Pandoc 的 Windows 桌面可视化文档转换工具。用户通过 QML 图形界面选择输入文件、输出格式和输出路径，软件在后台调用本机 Pandoc 完成转换。最终目标是逐步覆盖 Pandoc 支持的输入和输出格式互相转换。

第一版目标是先交付一个稳定可运行、可测试、可打包的文档转换 MVP，并逐步从 Markdown 转 HTML/DOCX/PDF 扩展到 Pandoc 支持的输入/输出格式互转，而不是一次性实现复杂编辑器或批量任务系统。

## 目标用户

- 经常使用 Markdown 写文档，但不想记忆 Pandoc 命令行参数的用户
- 需要把 Markdown 转换为 HTML、DOCX、PDF 的写作者、学生、技术文档维护者
- 需要把 Pandoc 支持的多种文档格式互相转换，但希望通过图形界面完成配置的用户
- 需要一个简单桌面工具来包装 Pandoc 的 Windows 用户

## 核心使用流程

1. 用户启动 md2any。
2. 软件检测本机 Pandoc 是否可用。
3. 用户选择一个输入文件。
4. 软件根据输入文件和输出格式生成默认输出路径。
5. 用户选择输入格式、输出格式和输出路径。
6. 用户点击开始转换。
7. 软件校验输入、输出和 Pandoc 环境。
8. 后端调用 Pandoc 执行转换。
9. UI 显示转换成功、失败、日志和错误信息。
10. 用户查看生成文件或根据错误提示调整后重试。

## MVP 功能列表

### 文件选择

- 选择单个输入文件
- 手动输入输入文件路径
- 选择输出文件路径
- 根据输出格式自动推断默认扩展名

### 转换配置

- 输入格式默认 `markdown`，可从当前 Pandoc 支持的可读格式中选择
- 输出格式从当前 Pandoc 支持的可写格式中选择
- 支持是否覆盖已存在输出文件

### Pandoc 调用

- 启动时检测 Pandoc 是否可用
- 支持默认从系统 `PATH` 查找 Pandoc
- 支持在 UI 中设置、检测并保存 Pandoc 可执行文件路径
- 支持读取 Pandoc 当前版本提供的输入/输出格式列表
- 使用后端模块集中构建 Pandoc 命令
- 执行真实转换并捕获标准输出、标准错误和退出码

### UI

- QML 主窗口
- 输入文件选择区域
- 输出格式选择区域
- 输出路径选择区域
- 转换按钮
- 状态提示区域
- 日志和错误信息区域

### 基础错误处理

- 未选择输入文件
- 输入文件不存在
- 输出路径为空
- 输出目录不存在
- 输出文件已存在且未允许覆盖
- Pandoc 未安装或不可用
- Pandoc 转换失败
- PDF 转换环境缺失时展示 Pandoc 错误信息

### 测试

- Pandoc 命令构建测试
- 输入输出参数校验测试
- 转换结果解析测试
- 配置读取和默认值测试

## 非 MVP 功能列表

### 后续可选功能

- 批量转换
- 最近文件和转换历史
- 自定义 Pandoc 参数
- 模板文件选择
- CSS 样式文件选择
- 元数据编辑
- PDF 引擎选择
- 打开输出文件或输出目录
- 动态列出 Pandoc 支持的输入格式和输出格式
- 支持 Pandoc 多格式互转
- 拖拽文件导入
- 多语言界面
- 主题切换

### 暂不实现功能

- 在线转换服务
- 用户账号系统
- 云端同步
- 插件系统
- 内置 Markdown 编辑器
- 所见即所得预览编辑器
- 复杂任务队列
- 跨平台安装器完整适配

## 页面和模块清单

### 页面

- 主转换页面：MVP 核心页面
- 设置弹窗：可在 MVP 后期加入，初版可先只显示 Pandoc 检测状态
- 关于页面：非 MVP

### 模块

- QML UI 模块
- C++ 应用入口
- QML 后端桥接模块
- Pandoc 执行模块
- Pandoc 格式发现模块
- 转换任务数据模型
- 配置读写模块
- 测试模块
- 打包脚本和说明

## 数据字段

### 转换任务

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| input_path | string | 输入 Markdown 文件路径 |
| output_path | string | 输出文件路径 |
| output_format | string | 输出格式，支持 html、docx、pdf |
| overwrite | boolean | 是否允许覆盖已存在输出文件 |
| extra_args | string[] | 额外 Pandoc 参数，MVP 默认为空 |
| input_format | string | 输入格式，默认 markdown，来自 Pandoc 支持的可读格式 |
| status | string | idle、running、success、failed |
| message | string | 当前状态说明或错误信息 |

### 转换结果

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| success | boolean | 是否转换成功 |
| command | string[] | 实际执行的 Pandoc 命令 |
| exit_code | integer | Pandoc 退出码 |
| stdout | string | 标准输出 |
| stderr | string | 标准错误 |
| output_path | string | 输出文件路径 |
| message | string | 面向用户的结果说明 |

### 应用配置

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| pandoc_path | string | Pandoc 可执行文件路径，默认使用 pandoc |
| last_input_dir | string | 上次输入目录 |
| last_output_dir | string | 上次输出目录 |
| last_input_format | string | 上次输入格式 |
| last_output_format | string | 上次输出格式 |

## 验收标准

- 软件可在 Windows 本地启动
- UI 使用 QML 实现
- 可以选择一个 Markdown 文件
- 可以选择 `html`、`docx`、`pdf` 输出格式
- 可以设置输出路径
- 可以设置并保存 Pandoc 路径
- 可以转换成功后打开输出文件或输出目录
- 点击转换后真实调用 Pandoc
- 转换成功后生成目标文件
- Pandoc 不存在时有明确错误提示
- 输入文件不存在时有明确错误提示
- 输出文件已存在时不会默认静默覆盖
- Pandoc 转换失败时显示错误详情
- 核心转换逻辑有自动化测试
- README 说明如何运行、测试和打包
