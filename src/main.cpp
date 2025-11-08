#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "orbit/OrbitCalculator.h"
#include "orbit/OrbitPath.h"
#include "data/TLEParser.h"
#include "data/SGP4Propagator.h"

int main(int argc, char *argv[])
{
    // ============================================
    // TEST DU PARSER TLE + SGP4
    // ============================================

    qDebug() << "";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "🧪 TEST COMPLET SGP4 + libsgp4";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "";

    // TLE réel de SPOT 7 (satellite français d'observation)
    QString line0 = "ISS ZARYA";
    QString line1 = "1 25544U 98067A   25308.55131963  .00010237  00000+0  18874-3 0  9994";
    QString line2 = "2 25544  51.6336 331.5320 0005028  16.6774 343.4380 15.49747070536934";

    // Parser le TLE
    TLEData tle = TLEParser::parseTLE(line0, line1, line2);

    qDebug() << "📡 Satellite:" << tle.name;
    qDebug() << "🆔 NORAD ID:" << tle.noradId;
    qDebug() << "📅 Époque:" << tle.epoch.toString("yyyy-MM-dd HH:mm:ss UTC");
    qDebug() << "📐 Inclinaison:" << tle.inclination << "°";
    qDebug() << "🌍 Altitude:" << QString::number(tle.altitude, 'f', 1) << "km";
    qDebug() << "⏱️  Période:" << QString::number(tle.period, 'f', 2) << "min";
    qDebug() << "🎯 Excentricité:" << QString::number(tle.eccentricity, 'f', 6);
    qDebug() << "";

    // Initialiser le propagateur SGP4
    SGP4Propagator propagator;
    if (!propagator.initialize(tle)) {
        qCritical() << "❌ Échec initialisation SGP4";
        return -1;
    }

    qDebug() << "";
    qDebug() << "🔄 === SIMULATION D'UNE ORBITE COMPLÈTE ===";
    qDebug() << "";

    // Test sur une orbite complète (9 points pour faire le tour complet)
    double periodSeconds = tle.period * 60.0;
    QDateTime startTime = tle.epoch;

    qDebug() << QString("%-10s %-20s %-12s %-12s %-12s %-10s")
                    .arg("Temps")
                    .arg("Date/Heure")
                    .arg("X (km)")
                    .arg("Y (km)")
                    .arg("Z (km)")
                    .arg("Dist (km)");
    qDebug() << QString("-").repeated(90);

    for (int i = 0; i <= 8; i++) {
        double t = (periodSeconds * i) / 8.0;
        QDateTime currentTime = startTime.addSecs(static_cast<qint64>(t));

        QVector3D pos = propagator.getPositionECI(currentTime);
        double distance = pos.length();

        qDebug() << QString("t+%1min  %2  %3  %4  %5  %6")
                        .arg(t/60.0, 6, 'f', 1)
                        .arg(currentTime.toString("HH:mm:ss"))
                        .arg(pos.x(), 9, 'f', 1)
                        .arg(pos.y(), 9, 'f', 1)
                        .arg(pos.z(), 9, 'f', 1)
                        .arg(distance, 8, 'f', 1);
    }

    qDebug() << "";
    qDebug() << "🎯 === TEST POSITION + VITESSE ===";
    qDebug() << "";

    QVector3D position, velocity;
    if (propagator.propagate(startTime, position, velocity)) {
        double speed = velocity.length();
        double altitudeCalc = position.length() - 6371.0;  // Rayon terrestre

        qDebug() << "📍 Position ECI (à l'époque):";
        qDebug() << "   X =" << QString::number(position.x(), 'f', 3) << "km";
        qDebug() << "   Y =" << QString::number(position.y(), 'f', 3) << "km";
        qDebug() << "   Z =" << QString::number(position.z(), 'f', 3) << "km";
        qDebug() << "   Distance au centre =" << QString::number(position.length(), 'f', 2) << "km";
        qDebug() << "   Altitude ≈" << QString::number(altitudeCalc, 'f', 1) << "km";
        qDebug() << "";
        qDebug() << "🚀 Vitesse ECI:";
        qDebug() << "   Vx =" << QString::number(velocity.x(), 'f', 3) << "km/s";
        qDebug() << "   Vy =" << QString::number(velocity.y(), 'f', 3) << "km/s";
        qDebug() << "   Vz =" << QString::number(velocity.z(), 'f', 3) << "km/s";
        qDebug() << "   Vitesse totale =" << QString::number(speed, 'f', 3) << "km/s";
        qDebug() << "";

        // Conversion pour affichage 3D
        QVector3D displayPos = SGP4Propagator::eciToDisplay(position);
        qDebug() << "🎨 Position pour Qt Quick 3D:";
        qDebug() << "   X =" << QString::number(displayPos.x(), 'f', 2);
        qDebug() << "   Y =" << QString::number(displayPos.y(), 'f', 2);
        qDebug() << "   Z =" << QString::number(displayPos.z(), 'f', 2);
        qDebug() << "   Distance =" << QString::number(displayPos.length(), 'f', 2) << "unités Qt";
    }

    qDebug() << "";
    qDebug() << "✅ Test SGP4 terminé avec succès !";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "";

    // ============================================
    // INITIALISATION APPLICATION Qt
    // ============================================

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // === Création des objets C++ pour QML ===
    OrbitCalculator orbitCalculator;
    OrbitPath orbitPath;

    // ⚠️ IMPORTANT: Synchroniser les paramètres orbitaux
    // Ces valeurs doivent correspondre à celles dans OrbitCalculator::getSatellitePosition()
    double semiMajorAxis = 500.0;
    double eccentricity = 0.3;
    double inclination = 45.0;

    orbitPath.setSemiMajorAxis(semiMajorAxis);
    orbitPath.setEccentricity(eccentricity);
    orbitPath.setInclination(inclination);
    orbitPath.setResolution(256);  // Plus de points = ligne plus continue

    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "🎬 Démarrage de l'application 3D";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "";
    qDebug() << "Configuration orbite de démonstration:";
    qDebug() << "  - Demi-grand axe:" << semiMajorAxis << "km";
    qDebug() << "  - Excentricité:" << eccentricity;
    qDebug() << "  - Inclinaison:" << inclination << "°";
    qDebug() << "  - Résolution:" << 256 << "points";
    qDebug() << "";

    // === Exposition à QML - IMPORTANT: faire AVANT de charger le QML ===
    engine.rootContext()->setContextProperty("orbitCalculator", &orbitCalculator);
    engine.rootContext()->setContextProperty("orbitPath", &orbitPath);

    // === Chargement du QML ===
    const QUrl url(QStringLiteral("qrc:/res/qml/main.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qWarning() << "❌ Erreur: impossible de charger" << url;
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
        );

    engine.load(url);

    // Vérification du chargement
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "❌ Erreur: aucun objet racine chargé!";
        return -1;
    }

    qDebug() << "✅ Application Qt démarrée avec succès";
    qDebug() << "";

    return app.exec();
}
