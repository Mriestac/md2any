#include "app_controller.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("md2any"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("md2any"));

    QQmlApplicationEngine engine;
    AppController appController;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &appController);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("Md2Any"), QStringLiteral("Main"));

    return app.exec();
}
