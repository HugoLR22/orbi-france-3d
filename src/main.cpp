#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "orbit/OrbitCalculator.h"
#include "orbit/OrbitPath.h"
#include "data/TLEParser.h"

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

    //TEST TLE ISS
    // Test du parser TLE
    QString line0 = "ISS (ZARYA)";
    QString line1 = "1 25544U 98067A   25308.55131963  .00010237  00000+0  18874-3 0  9994";
    QString line2 = "2 25544  51.6336 331.5320 0005028  16.6774 343.4380 15.49747070536934";

    try {
        TLEData tle = TLEParser::parseTLE(line0, line1, line2);

        qDebug() << "=== TLE PARSÉ ===";
        qDebug() << "Nom:" << tle.name;
        qDebug() << "NORAD ID:" << tle.noradId;
        qDebug() << "Inclinaison:" << tle.inclination << "°";
        qDebug() << "Altitude:" << tle.altitude << "km";
        qDebug() << "Période:" << tle.period << "min";

    } catch (QString error) {
        qCritical() << "Erreur parsing:" << error;
    }

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
