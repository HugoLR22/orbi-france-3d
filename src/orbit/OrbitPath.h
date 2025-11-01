#ifndef ORBITPATH_H
#define ORBITPATH_H

#include <QObject>
#include <QVector3D>
#include <QVariantList>

/**
 * @brief Génère les points de trajectoire orbitale pour visualisation 3D
 *
 * Cette classe calcule une série de points représentant une orbite complète
 * basée sur les paramètres orbitaux (Kepler). Utilisée pour dessiner
 * des anneaux de trajectoire dans Qt Quick 3D.
 */
class OrbitPath : public QObject
{
    Q_OBJECT

    // Propriétés exposées à QML
    Q_PROPERTY(double semiMajorAxis READ semiMajorAxis WRITE setSemiMajorAxis NOTIFY semiMajorAxisChanged)
    Q_PROPERTY(double eccentricity READ eccentricity WRITE setEccentricity NOTIFY eccentricityChanged)
    Q_PROPERTY(double inclination READ inclination WRITE setInclination NOTIFY inclinationChanged)
    Q_PROPERTY(int resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)

public:
    explicit OrbitPath(QObject *parent = nullptr);

    // Getters
    double semiMajorAxis() const { return m_semiMajorAxis; }
    double eccentricity() const { return m_eccentricity; }
    double inclination() const { return m_inclination; }
    int resolution() const { return m_resolution; }

    // Setters
    void setSemiMajorAxis(double value);
    void setEccentricity(double value);
    void setInclination(double value);
    void setResolution(int value);

    /**
     * @brief Génère les points de l'orbite complète
     * @return Liste de QVector3D convertis en QVariantList pour QML
     */
    Q_INVOKABLE QVariantList generateOrbitPoints();

signals:
    void semiMajorAxisChanged();
    void eccentricityChanged();
    void inclinationChanged();
    void resolutionChanged();
    void orbitChanged(); // Signal global quand l'orbite change

private:
    double m_semiMajorAxis = 500.0;  // Demi-grand axe (rayon orbital moyen)
    double m_eccentricity = 0.3;     // Excentricité (0 = cercle, <1 = ellipse)
    double m_inclination = 45.0;     // Inclinaison en degrés
    int m_resolution = 128;          // Nombre de points pour tracer l'orbite

    /**
     * @brief Calcule un point de l'orbite à un angle donné
     * @param angle Anomalie vraie en radians
     * @return Position 3D du point
     */
    QVector3D calculateOrbitPoint(double angle) const;
};

#endif // ORBITPATH_H
