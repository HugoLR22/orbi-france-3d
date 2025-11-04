#include "TLEParser.h"
#include <QtMath>
#include <QDebug>

// Constantes physiques
const double EARTH_RADIUS_KM = 6371.0;       // Rayon moyen de la Terre
const double MU = 398600.4418;               // Constante gravitationnelle (km³/s²)
const double MINUTES_PER_DAY = 1440.0;       // Minutes dans une journée

TLEData TLEParser::parseTLE(const QString& line0, const QString& line1, const QString& line2)
{
    TLEData tle = parseTLE(line1, line2);
    tle.name = line0.trimmed();
    return tle;
}

TLEData TLEParser::parseTLE(const QString& line1, const QString& line2)
{
    TLEData tle;

    // Vérification des checksums
    if (!verifyChecksum(line1)) {
        qWarning() << "❌ Checksum invalide ligne 1:" << line1;
    }
    if (!verifyChecksum(line2)) {
        qWarning() << "❌ Checksum invalide ligne 2:" << line2;
    }

    // === PARSING LIGNE 1 ===

    // Numéro NORAD (colonnes 3-7)
    tle.noradId = extractInt(line1, 2, 5);

    // Classification (colonne 8) - ignoré pour l'instant

    // Désignation internationale (colonnes 10-17)
    tle.internationalDesignator = line1.mid(9, 8).trimmed();

    // Époque (colonnes 19-32)
    int epochYearShort = extractInt(line1, 18, 2);
    tle.epochDay = extractDouble(line1, 20, 12);

    // Conversion année : 00-56 = 2000-2056, 57-99 = 1957-1999
    tle.epochYear = (epochYearShort < 57) ? 2000 + epochYearShort : 1900 + epochYearShort;
    tle.epoch = epochToDateTime(epochYearShort, tle.epochDay);

    // 1ère dérivée du mouvement moyen (colonnes 34-43)
    tle.meanMotionDot = extractDouble(line1, 33, 10);

    // 2ème dérivée du mouvement moyen (colonnes 45-52, format compact)
    QString dotDotStr = line1.mid(44, 8).trimmed();
    tle.meanMotionDotDot = parseScientific(dotDotStr);

    // B* coefficient de traînée (colonnes 54-61, format compact)
    QString bstarStr = line1.mid(53, 8).trimmed();
    tle.bstar = parseScientific(bstarStr);

    // Numéro du jeu d'éléments (colonnes 65-68)
    tle.elementSetNumber = extractInt(line1, 64, 4);

    // === PARSING LIGNE 2 ===

    // Inclinaison (colonnes 9-16)
    tle.inclination = extractDouble(line2, 8, 8);

    // RAAN - Right Ascension of Ascending Node (colonnes 18-25)
    tle.raan = extractDouble(line2, 17, 8);

    // Excentricité (colonnes 27-33, format implicite 0.xxxxxxx)
    double eccRaw = extractDouble(line2, 26, 7);
    tle.eccentricity = eccRaw / 10000000.0;  // Ajouter le point décimal

    // Argument du périgée (colonnes 35-42)
    tle.argOfPerigee = extractDouble(line2, 34, 8);

    // Anomalie moyenne (colonnes 44-51)
    tle.meanAnomaly = extractDouble(line2, 43, 8);

    // Mouvement moyen (colonnes 53-63)
    tle.meanMotion = extractDouble(line2, 52, 11);

    // Numéro de révolution (colonnes 64-68)
    tle.revolutionNumber = extractInt(line2, 63, 5);

    // === CALCUL DES PARAMÈTRES DÉRIVÉS ===
    tle.calculateDerivedParameters();

    return tle;
}

void TLEData::calculateDerivedParameters()
{
    // Période orbitale (minutes)
    period = MINUTES_PER_DAY / meanMotion;

    // Calcul du demi-grand axe via la 3ème loi de Kepler
    // T² = (4π²/μ) × a³
    // a³ = (μ × T²) / (4π²)

    double periodSeconds = period * 60.0;  // Conversion en secondes
    double a3 = (MU * periodSeconds * periodSeconds) / (4.0 * M_PI * M_PI);
    semiMajorAxis = qPow(a3, 1.0/3.0);

    // Altitude moyenne (approximation)
    altitude = semiMajorAxis - EARTH_RADIUS_KM;

    // Log pour debug
    qDebug() << "📊 Paramètres calculés:";
    qDebug() << "   Période:" << period << "min";
    qDebug() << "   Demi-grand axe:" << semiMajorAxis << "km";
    qDebug() << "   Altitude:" << altitude << "km";
}

bool TLEParser::verifyChecksum(const QString& line)
{
    if (line.length() < 69) {
        return false;
    }

    int checksum = 0;

    // Calcul du checksum sur les 68 premiers caractères
    for (int i = 0; i < 68; i++) {
        QChar c = line[i];

        if (c.isDigit()) {
            checksum += c.digitValue();
        } else if (c == '-') {
            checksum += 1;  // Le signe moins compte pour 1
        }
        // Les lettres et autres caractères ne comptent pas
    }

    checksum %= 10;  // Modulo 10

    // Comparaison avec le checksum déclaré (dernier caractère)
    int declaredChecksum = line[68].digitValue();

    return checksum == declaredChecksum;
}

double TLEParser::extractDouble(const QString& line, int start, int length)
{
    QString substr = line.mid(start, length).trimmed();
    bool ok;
    double value = substr.toDouble(&ok);

    if (!ok) {
        qWarning() << "Erreur extraction double:" << substr << "à la position" << start;
        return 0.0;
    }

    return value;
}

int TLEParser::extractInt(const QString& line, int start, int length)
{
    QString substr = line.mid(start, length).trimmed();
    bool ok;
    int value = substr.toInt(&ok);

    if (!ok) {
        qWarning() << "Erreur extraction int:" << substr << "à la position" << start;
        return 0;
    }

    return value;
}

QDateTime TLEParser::epochToDateTime(int year, double dayOfYear)
{
    // Conversion année courte (YY) en année complète (YYYY)
    int fullYear = (year < 57) ? 2000 + year : 1900 + year;

    // Créer une date au 1er janvier de l'année
    QDateTime epoch(QDate(fullYear, 1, 1), QTime(0, 0, 0), Qt::UTC);

    // Ajouter le nombre de jours (partie entière)
    int wholeDays = static_cast<int>(dayOfYear) - 1;  // -1 car le jour 1 = 1er janvier
    epoch = epoch.addDays(wholeDays);

    // Ajouter la fraction de jour en millisecondes
    double fractionOfDay = dayOfYear - static_cast<int>(dayOfYear);
    qint64 milliseconds = static_cast<qint64>(fractionOfDay * 86400000.0);  // 86400000 ms par jour
    epoch = epoch.addMSecs(milliseconds);

    return epoch;
}

double TLEParser::parseScientific(const QString& str)
{
    if (str.isEmpty()) {
        return 0.0;
    }

    // Format TLE compact : "12345-3" signifie 0.12345 × 10^-3
    // Format TLE compact : " 12345-3" avec espace = positif
    // Format TLE compact : "-12345-3" avec signe = négatif

    QString cleaned = str.trimmed();

    // Séparer mantisse et exposant
    int expPos = -1;
    for (int i = 1; i < cleaned.length(); i++) {  // Commence à 1 pour ignorer le signe éventuel
        if (cleaned[i] == '-' || cleaned[i] == '+') {
            expPos = i;
            break;
        }
    }

    if (expPos == -1) {
        // Pas d'exposant, valeur directe
        return cleaned.toDouble();
    }

    // Extraire mantisse et exposant
    QString mantissaStr = cleaned.left(expPos);
    QString exponentStr = cleaned.mid(expPos);

    // Ajouter le point décimal implicite
    double mantissa = mantissaStr.toDouble() / 100000.0;  // 5 chiffres de mantisse
    int exponent = exponentStr.toInt();

    return mantissa * qPow(10.0, exponent);
}
