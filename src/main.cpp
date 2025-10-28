#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "orbit/OrbitCalculator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // Enregistrer la classe C++ pour accès depuis QML
    OrbitCalculator orbitCalculator;
    engine.rootContext()->setContextProperty("orbitCalculator", &orbitCalculator);

    const QUrl url(QStringLiteral("qrc:/res/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
