#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    const QCoreApplication app(argc, argv);
    const auto args = app.arguments();

    if (args.contains(QStringLiteral("--fail"))) {
        QTextStream(stderr) << "fake pandoc failure\n";
        return 7;
    }

    if (args.contains(QStringLiteral("--list-input-formats"))) {
        QTextStream(stdout) << "markdown\nhtml\ndocx\n";
        return 0;
    }

    if (args.contains(QStringLiteral("--list-output-formats"))) {
        QTextStream(stdout) << "html\ndocx\npdf\nepub\n";
        return 0;
    }

    const auto outputIndex = args.indexOf(QStringLiteral("-o"));
    if (outputIndex < 0 || outputIndex + 1 >= args.size()) {
        QTextStream(stderr) << "missing output path\n";
        return 2;
    }

    const auto outputPath = args.at(outputIndex + 1);

    if (outputPath.contains(QStringLiteral("fail"))) {
        QTextStream(stderr) << "fake pandoc failure\n";
        return 7;
    }

    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream(stderr) << "failed to write output\n";
        return 3;
    }

    outputFile.write("converted by fake pandoc\n");
    return 0;
}
