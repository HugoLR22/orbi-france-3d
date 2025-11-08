#ifndef SGP4PROPAGATOR_H
#define SGP4PROPAGATOR_H

#include <QObject>
#include <QVector3D>
#include <QDateTime>
#include "TLEParser.h"

// Includes complets de libsgp4
#include "SGP4.h"
#include "Tle.h"
#include "DateTime.h"
#include "Eci.h"

/**
 * @brief Wrapper Qt-friendly pour libsgp4
 *
 * Propage les orbites satellites à partir des éléments TLE
 * en utilisant la vraie bibliothèque SGP4 (précision sub-kilométrique)
 */
class SGP4Propagator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString satelliteName READ satelliteName WRITE setSatelliteName NOTIFY satelliteNameChanged)
    Q_PROPERTY(double altitude READ altitude NOTIFY positionChanged)
    Q_PROPERTY(double velocity READ velocity NOTIFY positionChanged)

public:
    explicit SGP4Propagator(QObject *parent = nullptr);
    ~SGP4Propagator();

    /**
     * @brief Initialise le propagateur avec des données TLE
     * @param tle Structure TLEData parsée
     * @return true si l'initialisation réussit
     */
    bool initialize(const TLEData& tle);

    /**
     * @brief Calcule la position du satellite à un instant donné
     * @param dateTime Date/heure pour laquelle calculer la position (UTC)
     * @return Position en coordonnées ECI (Earth-Centered Inertial) en km
     */
    Q_INVOKABLE QVector3D getPositionECI(const QDateTime& dateTime) const;

    /**
     * @brief Calcule la position relative à l'époque TLE (en secondes)
     * @param secondsSinceEpoch Secondes depuis l'époque du TLE
     * @return Position ECI en km
     */
    Q_INVOKABLE QVector3D getPositionAtTime(double secondsSinceEpoch) const;

    /**
     * @brief Calcule position ET vitesse
     * @param dateTime Date/heure (UTC)
     * @param position [out] Position ECI en km
     * @param velocity [out] Vitesse ECI en km/s
     * @return true si le calcul réussit
     */
    bool propagate(const QDateTime& dateTime, QVector3D& position, QVector3D& velocity) const;

    /**
     * @brief Vérifie si le propagateur est initialisé
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Convertit ECI vers coordonnées pour affichage Qt Quick 3D
     * @param eci Position en coordonnées ECI (km)
     * @param scale Facteur d'échelle pour l'affichage (défaut: 1.0)
     * @return Position adaptée pour Qt Quick 3D
     */
    static QVector3D eciToDisplay(const QVector3D& eci, double scale = 1.0);

    // Getters
    QString satelliteName() const { return m_satelliteName; }
    double altitude() const;
    double velocity() const;
    const TLEData& tleData() const { return m_tle; }

    // Setters
    void setSatelliteName(const QString& name);

signals:
    void satelliteNameChanged();
    void positionChanged();
    void propagationError(const QString& error);

private:
    bool m_initialized;
    QString m_satelliteName;
    TLEData m_tle;

    // Objets libsgp4
    libsgp4::SGP4* m_sgp4;
    libsgp4::Tle* m_tleObj;

    /**
     * @brief Convertit QDateTime vers DateTime de libsgp4
     */
    libsgp4::DateTime qDateTimeToSGP4(const QDateTime& dt) const;

    /**
     * @brief Calcule le temps en minutes depuis l'époque TLE
     */
    double minutesSinceEpoch(const QDateTime& dateTime) const;
};

#endif // SGP4PROPAGATOR_H
