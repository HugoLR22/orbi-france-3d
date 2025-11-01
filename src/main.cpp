#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "orbit/OrbitCalculator.h"
#include "orbit/OrbitPath.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // === Création des objets C++ ===
    OrbitCalculator orbitCalculator;
    OrbitPath orbitPath;

    // Synchroniser les paramètres orbitaux entre les deux classes
    orbitPath.setSemiMajorAxis(500.0);
    orbitPath.setEccentricity(0.3);
    orbitPath.setInclination(45.0);
    orbitPath.setResolution(2048);

    // === Exposition à QML ===
    engine.rootContext()->setContextProperty("orbitCalculator", &orbitCalculator);
    engine.rootContext()->setContextProperty("orbitPath", &orbitPath);

    // === Chargement du QML ===
    const QUrl url(QStringLiteral("qrc:/res/qml/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection
        );

    engine.load(url);

    return app.exec();
}
