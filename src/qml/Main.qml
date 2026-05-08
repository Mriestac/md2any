import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: root

    width: 920
    height: 640
    minimumWidth: 780
    minimumHeight: 540
    visible: true
    title: "md2any"
    color: "#f6f7f9"

    function pathFromUrl(url) {
        var text = String(url)
        if (text.startsWith("file:///")) {
            return decodeURIComponent(text.substring(8))
        }
        if (text.startsWith("file:/")) {
            return decodeURIComponent(text.substring(6))
        }
        return decodeURIComponent(text)
    }

    function folderUrl(path) {
        if (path.length === 0) {
            return undefined
        }
        return "file:///" + path.replace(/\\/g, "/")
    }

    function outputPathFor(inputPath, format) {
        if (inputPath.length === 0) {
            return ""
        }

        var dotIndex = inputPath.lastIndexOf(".")
        var basePath = dotIndex > 0 ? inputPath.substring(0, dotIndex) : inputPath
        return basePath + "." + format
    }

    Component.onCompleted: {
        var index = outputFormatBox.model.indexOf(appController.defaultOutputFormat)
        if (index >= 0) {
            outputFormatBox.currentIndex = index
        }
    }

    FileDialog {
        id: inputDialog
        title: "选择 Markdown 文件"
        fileMode: FileDialog.OpenFile
        currentFolder: root.folderUrl(appController.lastInputDir)
        nameFilters: ["Markdown 文件 (*.md *.markdown)", "所有文件 (*)"]
        onAccepted: {
            inputPathField.text = root.pathFromUrl(selectedFile)
            if (outputPathField.text.length === 0) {
                outputPathField.text = root.outputPathFor(inputPathField.text, outputFormatBox.currentText)
            }
        }
    }

    FileDialog {
        id: outputDialog
        title: "选择输出文件"
        fileMode: FileDialog.SaveFile
        currentFolder: root.folderUrl(appController.lastOutputDir)
        nameFilters: ["HTML 文件 (*.html)", "Word 文件 (*.docx)", "PDF 文件 (*.pdf)", "所有文件 (*)"]
        onAccepted: {
            outputPathField.text = appController.normalizedOutputPath(
                root.pathFromUrl(selectedFile),
                outputFormatBox.currentText)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: "md2any"
                    color: "#20242a"
                    font.pixelSize: 26
                    font.bold: true
                }

                Label {
                    text: "基于 Pandoc 的文档转换工具"
                    color: "#69707a"
                    font.pixelSize: 13
                }
            }

            Label {
                text: appController.busy ? "转换中" : (appController.valid ? "就绪" : "待检查")
                color: appController.busy ? "#996c00" : (appController.valid ? "#1b7f45" : "#69707a")
                font.pixelSize: 14
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#dfe3e8"
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 10
            rowSpacing: 12

            Label {
                text: "输入文件"
                color: "#30363d"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            TextField {
                id: inputPathField
                Layout.fillWidth: true
                placeholderText: "选择或输入 .md 文件路径"
                selectByMouse: true
                enabled: !appController.busy
            }

            Button {
                text: "选择"
                enabled: !appController.busy
                onClicked: inputDialog.open()
            }

            Label {
                text: "输出文件"
                color: "#30363d"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            TextField {
                id: outputPathField
                Layout.fillWidth: true
                placeholderText: "选择或输入输出文件路径"
                selectByMouse: true
                enabled: !appController.busy
                onEditingFinished: outputPathField.text = appController.normalizedOutputPath(
                    outputPathField.text,
                    outputFormatBox.currentText)
            }

            Button {
                text: "保存到"
                enabled: !appController.busy
                onClicked: outputDialog.open()
            }

            Label {
                text: "输出格式"
                color: "#30363d"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            ComboBox {
                id: outputFormatBox
                model: ["html", "docx", "pdf"]
                Layout.preferredWidth: 160
                enabled: !appController.busy
                onCurrentTextChanged: {
                    if (outputPathField.text.length > 0) {
                        outputPathField.text = appController.normalizedOutputPath(outputPathField.text, currentText)
                    } else if (inputPathField.text.length > 0) {
                        outputPathField.text = root.outputPathFor(inputPathField.text, currentText)
                    }
                }
            }

            CheckBox {
                id: overwriteBox
                text: "允许覆盖"
                enabled: !appController.busy
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "检查配置"
                enabled: !appController.busy
                onClicked: appController.validateConversion(
                    inputPathField.text,
                    outputPathField.text,
                    outputFormatBox.currentText,
                    overwriteBox.checked)
            }

            Button {
                text: "开始转换"
                enabled: !appController.busy
                highlighted: true
                onClicked: appController.startConversion(
                    inputPathField.text,
                    outputPathField.text,
                    outputFormatBox.currentText,
                    overwriteBox.checked)
            }

            Button {
                text: "取消"
                enabled: appController.busy
                onClicked: appController.cancelConversion()
            }

            Button {
                text: "打开输出目录"
                enabled: !appController.busy
                onClicked: appController.openOutputDirectory()
            }

            Label {
                text: appController.statusMessage
                color: appController.valid ? "#1b7f45" : "#b42318"
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            indeterminate: appController.busy
            visible: appController.busy
        }

        Label {
            text: "日志"
            color: "#30363d"
            font.bold: true
        }

        TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
            readOnly: true
            wrapMode: TextEdit.Wrap
            text: appController.logText.length > 0 ? appController.logText : "检查和转换结果会显示在这里。"
            color: "#24292f"
            font.family: "Consolas"
            background: Rectangle {
                color: "#ffffff"
                border.color: "#d0d7de"
                radius: 6
            }
        }
    }
}
