#include "SGP4Propagator.h"
#include <QtMath>
#include <QDebug>

// Constantes physiques
const double EARTH_RADIUS_KM = 6371.0;

SGP4Propagator::SGP4Propagator(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_sgp4(nullptr)
    , m_tleObj(nullptr)
{
}

SGP4Propagator::~SGP4Propagator()
{
    delete m_sgp4;
    delete m_tleObj;
}

bool SGP4Propagator::initialize(const TLEData& tle)
{
    try {
        m_tle = tle;
        m_satelliteName = tle.name;

        // Nettoyer les objets précédents
        delete m_sgp4;
        delete m_tleObj;

        // Vérifier que les lignes TLE brutes sont disponibles
        if (tle.line1.isEmpty() || tle.line2.isEmpty()) {
            qCritical() << "❌ Lignes TLE brutes manquantes";
            m_initialized = false;
            return false;
        }

        // Créer l'objet TLE pour libsgp4
        std::string name = tle.name.toStdString();
        std::string line1 = tle.line1.toStdString();
        std::string line2 = tle.line2.toStdString();

        m_tleObj = new libsgp4::Tle(name, line1, line2);
        m_sgp4 = new libsgp4::SGP4(*m_tleObj);

        m_initialized = true;

        qDebug() << "✅ SGP4 (libsgp4) initialisé pour:" << m_satelliteName;
        qDebug() << "   Altitude:" << tle.altitude << "km";
        qDebug() << "   Inclinaison:" << tle.inclination << "°";
        qDebug() << "   Période:" << tle.period << "min";
        qDebug() << "   Utilise libsgp4 RÉELLE";

        return true;

    } catch (const std::exception& e) {
        qCritical() << "❌ Erreur initialisation SGP4:" << e.what();
        m_initialized = false;
        emit propagationError(QString("Erreur SGP4: %1").arg(e.what()));
        return false;
    }
}

void SGP4Propagator::setSatelliteName(const QString& name)
{
    if (m_satelliteName != name) {
        m_satelliteName = name;
        emit satelliteNameChanged();
    }
}

double SGP4Propagator::altitude() const
{
    if (!m_initialized) return 0.0;
    return m_tle.altitude;
}

double SGP4Propagator::velocity() const
{
    if (!m_initialized) return 0.0;

    // Vitesse orbitale circulaire : v = sqrt(μ/r)
    const double MU = 398600.4418;  // km³/s²
    double r = m_tle.semiMajorAxis;
    return qSqrt(MU / r);
}

libsgp4::DateTime SGP4Propagator::qDateTimeToSGP4(const QDateTime& dt) const
{
    QDateTime utc = dt.toUTC();

    int year = utc.date().year();
    int month = utc.date().month();
    int day = utc.date().day();
    int hour = utc.time().hour();
    int minute = utc.time().minute();
    int second = utc.time().second();
    int microsecond = utc.time().msec() * 1000;

    return libsgp4::DateTime(year, month, day, hour, minute, second, microsecond);
}

double SGP4Propagator::minutesSinceEpoch(const QDateTime& dateTime) const
{
    qint64 msecsSinceEpoch = m_tle.epoch.msecsTo(dateTime);
    return msecsSinceEpoch / 60000.0;
}

QVector3D SGP4Propagator::getPositionECI(const QDateTime& dateTime) const
{
    if (!m_initialized || !m_tleObj || !m_sgp4) {
        qWarning() << "❌ SGP4 non initialisé correctement";

        // Fallback sur calcul simplifié
        double secondsSince = minutesSinceEpoch(dateTime) * 60.0;
        return getPositionAtTime(secondsSince);
    }

    try {
        // Convertir QDateTime vers DateTime de libsgp4
        libsgp4::DateTime sgp4Time = qDateTimeToSGP4(dateTime);

        // Propager avec SGP4
        libsgp4::Eci eci = m_sgp4->FindPosition(sgp4Time);

        // Extraire la position
        libsgp4::Vector pos = eci.Position();

        return QVector3D(pos.x, pos.y, pos.z);

    } catch (const std::exception& e) {
        qWarning() << "❌ Erreur propagation SGP4:" << e.what();
        emit const_cast<SGP4Propagator*>(this)->propagationError(QString(e.what()));
        return QVector3D(0, 0, 0);
    }
}

QVector3D SGP4Propagator::getPositionAtTime(double secondsSinceEpoch) const
{
    if (!m_initialized) {
        return QVector3D(0, 0, 0);
    }

    // Calculer la date/heure correspondante
    QDateTime targetTime = m_tle.epoch.addSecs(static_cast<qint64>(secondsSinceEpoch));

    return getPositionECI(targetTime);
}

bool SGP4Propagator::propagate(const QDateTime& dateTime, QVector3D& position, QVector3D& velocity) const
{
    if (!m_initialized) {
        emit const_cast<SGP4Propagator*>(this)->propagationError("SGP4 non initialisé");
        return false;
    }

    if (!m_tleObj || !m_sgp4) {
        // Fallback : calcul position uniquement
        position = getPositionECI(dateTime);

        // Vitesse par différence finie
        QDateTime dt1 = dateTime.addSecs(1);
        QVector3D pos1 = getPositionECI(dt1);
        velocity = pos1 - position;

        return true;
    }

    try {
        libsgp4::DateTime sgp4Time = qDateTimeToSGP4(dateTime);
        libsgp4::Eci eci = m_sgp4->FindPosition(sgp4Time);

        libsgp4::Vector pos = eci.Position();
        libsgp4::Vector vel = eci.Velocity();

        position = QVector3D(pos.x, pos.y, pos.z);
        velocity = QVector3D(vel.x, vel.y, vel.z);

        return true;

    } catch (const std::exception& e) {
        qWarning() << "❌ Erreur propagation:" << e.what();
        emit const_cast<SGP4Propagator*>(this)->propagationError(QString(e.what()));
        return false;
    }
}

QVector3D SGP4Propagator::eciToDisplay(const QVector3D& eci, double scale)
{
    // ECI est en kilomètres, centré sur la Terre
    // Conversion pour Qt Quick 3D où la Terre a scale=3

    // Si Terre scale=3 et rayon réel=6371 km
    // alors 1 unité Qt = 6371/3 = 2123.67 km
    double qtUnitToKm = EARTH_RADIUS_KM / 3.0;

    return QVector3D(
        eci.x() / qtUnitToKm * scale,
        eci.y() / qtUnitToKm * scale,
        eci.z() / qtUnitToKm * scale
        );
}
