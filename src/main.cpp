#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "orbit/OrbitCalculator.h"
#include "orbit/OrbitPath.h"
#include "data/TLEParser.h"
#include "data/SGP4Propagator.h"
#include "data/SatelliteDatabase.h"

int main(int argc, char *argv[])
{
    // ============================================
    // TEST DE LA BASE DE DONNÉES SATELLITES
    // ============================================

    qDebug() << "";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "🧪 TEST SATELLITEDATABASE";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";

    SatelliteDatabase database;
    database.loadFrenchSatellites();

    qDebug() << "";
    qDebug() << "📊 Statistiques:";
    qDebug() << "  - Nombre total:" << database.count() << "satellites";
    qDebug() << "  - Catégories:" << database.categories();
    qDebug() << "";

    // Lister tous les satellites
    qDebug() << "📡 Liste des satellites:";
    QVariantList allSats = database.getAllSatellites();
    for (const QVariant& satVar : allSats) {
        QVariantMap sat = satVar.toMap();
        qDebug() << QString("  - %1 (%2) - %3 km - %4")
                        .arg(sat["name"].toString(), -20)
                        .arg(sat["category"].toString(), -15)
                        .arg(sat["altitude"].toDouble(), 7, 'f', 1)
                        .arg(sat["color"].toString());
    }

    qDebug() << "";
    qDebug() << "🎯 Test calcul positions à l'époque:";
    QDateTime testTime = QDateTime::currentDateTimeUtc();
    QVariantList positions = database.calculateAllPositions(testTime);

    for (const QVariant& posVar : positions) {
        QVariantMap pos = posVar.toMap();
        qDebug() << QString("  %1: pos(%2, %3, %4) alt=%5 km")
                        .arg(pos["name"].toString(), -20)
                        .arg(pos["x"].toDouble(), 6, 'f', 2)
                        .arg(pos["y"].toDouble(), 6, 'f', 2)
                        .arg(pos["z"].toDouble(), 6, 'f', 2)
                        .arg(pos["altitude"].toDouble(), 7, 'f', 1);
    }

    qDebug() << "";
    qDebug() << "✅ Test SatelliteDatabase terminé avec succès !";
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

    // Configuration orbite de démonstration (pour l'ancien système)
    double semiMajorAxis = 500.0;
    double eccentricity = 0.3;
    double inclination = 45.0;

    orbitPath.setSemiMajorAxis(semiMajorAxis);
    orbitPath.setEccentricity(eccentricity);
    orbitPath.setInclination(inclination);
    orbitPath.setResolution(256);

    // === BASE DE DONNÉES SATELLITES (NOUVEAU) ===
    SatelliteDatabase* satelliteDB = new SatelliteDatabase(&app);
    satelliteDB->loadFrenchSatellites();

    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "🎬 Démarrage de l'application 3D";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "";
    qDebug() << "🛰️  Base de données:";
    qDebug() << "  -" << satelliteDB->count() << "satellites français chargés";
    qDebug() << "  - Catégories:" << satelliteDB->categories();
    qDebug() << "";

    // === Exposition à QML ===
    engine.rootContext()->setContextProperty("orbitCalculator", &orbitCalculator);
    engine.rootContext()->setContextProperty("orbitPath", &orbitPath);
    engine.rootContext()->setContextProperty("satelliteDatabase", satelliteDB);  // NOUVEAU

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
