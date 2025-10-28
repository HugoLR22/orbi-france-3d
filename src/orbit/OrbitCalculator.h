#ifndef ORBITCALCULATOR_H
#define ORBITCALCULATOR_H

#include <QObject>
#include <QVector3D>

class OrbitCalculator : public QObject
{
    Q_OBJECT
public:
    explicit OrbitCalculator(QObject *parent = nullptr);

    Q_INVOKABLE QVector3D getSatellitePosition(double timeSeconds);

signals:
    void satellitePositionChanged(QVector3D newPos);
};

#endif // ORBITCALCULATOR_H
