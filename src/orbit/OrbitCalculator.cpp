#include "OrbitCalculator.h"
#include <QtMath>

OrbitCalculator::OrbitCalculator(QObject *parent) : QObject(parent)
{
}

QVector3D OrbitCalculator::getSatellitePosition(double timeSeconds)
{
    // Simple orbite circulaire fictive autour de la Terre
    double radius = 500; // distance arbitraire
    double speed = 0.2;  // vitesse angulaire
    double angle = speed * timeSeconds;

    float x = radius * qCos(angle);
    float y = radius * qSin(angle);
    float z = 0.5 * qSin(angle * 0.5);

    return QVector3D(x, y, z);
}
