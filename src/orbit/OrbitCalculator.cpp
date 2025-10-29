#include "OrbitCalculator.h"
#include <QtMath>
#include <QVector3D>

OrbitCalculator::OrbitCalculator(QObject *parent)
    : QObject(parent)
{
}

QVector3D OrbitCalculator::getSatellitePosition(double timeSeconds)
{
    // === Paramètres orbitaux ===
    double semiMajorAxis = 500.0;   // "taille" de l'orbite (demi-grand axe)
    double eccentricity = 0.3;     // 0 = cercle parfait, 0.3 = orbite elliptique
    double inclination = 45.0;     // inclinaison de l'orbite en degrés
    double angularSpeed = 0.3;     // vitesse orbitale rad/s

    // === Calcul de la position dans le plan orbital ===
    double angle = angularSpeed * timeSeconds; // position sur l’ellipse
    double radius = semiMajorAxis * (1 - eccentricity * eccentricity)
                    / (1 + eccentricity * qCos(angle)); // équation de l'ellipse

    double x_orb = radius * qCos(angle);
    double y_orb = radius * qSin(angle);
    double z_orb = 0.0;

    // === Application de l’inclinaison (rotation autour de l’axe X) ===
    double incRad = qDegreesToRadians(inclination);
    double y_rot = y_orb * qCos(incRad) - z_orb * qSin(incRad);
    double z_rot = y_orb * qSin(incRad) + z_orb * qCos(incRad);

    // === Retourne la position 3D du satellite ===
    return QVector3D(x_orb, y_rot, z_rot);
}
