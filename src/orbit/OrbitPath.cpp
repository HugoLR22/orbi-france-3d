#include "OrbitPath.h"
#include <QtMath>

OrbitPath::OrbitPath(QObject *parent)
    : QObject(parent)
{
}

void OrbitPath::setSemiMajorAxis(double value)
{
    if (qFuzzyCompare(m_semiMajorAxis, value))
        return;

    m_semiMajorAxis = value;
    emit semiMajorAxisChanged();
    emit orbitChanged();
}

void OrbitPath::setEccentricity(double value)
{
    if (qFuzzyCompare(m_eccentricity, value))
        return;

    // Limite l'excentricité à [0, 0.99] pour éviter les orbites hyperboliques
    m_eccentricity = qBound(0.0, value, 0.99);
    emit eccentricityChanged();
    emit orbitChanged();
}

void OrbitPath::setInclination(double value)
{
    if (qFuzzyCompare(m_inclination, value))
        return;

    m_inclination = value;
    emit inclinationChanged();
    emit orbitChanged();
}

void OrbitPath::setResolution(int value)
{
    if (m_resolution == value)
        return;

    // Limite la résolution entre 32 et 512 points
    m_resolution = qBound(32, value, 512);
    emit resolutionChanged();
    emit orbitChanged();
}

QVector3D OrbitPath::calculateOrbitPoint(double angle) const
{
    // === Équation polaire de l'ellipse (formule de Kepler) ===
    // r = a(1 - e²) / (1 + e·cos(θ))
    double radius = m_semiMajorAxis * (1.0 - m_eccentricity * m_eccentricity)
                    / (1.0 + m_eccentricity * qCos(angle));

    // Position dans le plan orbital (XY)
    double x_orb = radius * qCos(angle);
    double y_orb = radius * qSin(angle);
    double z_orb = 0.0;

    // === Rotation pour l'inclinaison (autour de l'axe X) ===
    double incRad = qDegreesToRadians(m_inclination);
    double cosInc = qCos(incRad);
    double sinInc = qSin(incRad);

    double y_rot = y_orb * cosInc - z_orb * sinInc;
    double z_rot = y_orb * sinInc + z_orb * cosInc;

    return QVector3D(x_orb, y_rot, z_rot);
}

QVariantList OrbitPath::generateOrbitPoints()
{
    QVariantList points;
    points.reserve(m_resolution + 1);

    QVector3D firstPoint;

    // Génère les points sur 360° (une orbite complète)
    for (int i = 0; i <= m_resolution; ++i)  // <= pour fermer la boucle
    {
        double angle = (2.0 * M_PI * i) / m_resolution;
        QVector3D point = calculateOrbitPoint(angle);

        if (i == 0)
            firstPoint = point;

        // Convertir QVector3D en QVariantMap pour QML
        QVariantMap pointMap;
        pointMap["x"] = point.x();
        pointMap["y"] = point.y();
        pointMap["z"] = point.z();

        points.append(pointMap);
    }

    return points;
}
