#ifndef TLEPARSER_H
#define TLEPARSER_H

#include <QString>
#include <QDateTime>

/**
 * @brief Structure contenant les éléments orbitaux TLE
 *
 * Format TLE (Two-Line Element) : standard NORAD/NASA
 * pour décrire les orbites de satellites
 */
struct TLEData {
    // === Identification ===
    QString name;               // Nom du satellite
    int noradId;               // Numéro NORAD (identification unique)
    QString internationalDesignator;  // Désignation internationale

    // === Lignes TLE brutes (pour libsgp4) ===
    QString line0;             // Ligne 0 : nom
    QString line1;             // Ligne 1 : paramètres généraux
    QString line2;             // Ligne 2 : éléments orbitaux

    // === Époque ===
    QDateTime epoch;           // Date/heure de référence des éléments
    double epochYear;          // Année de l'époque
    double epochDay;           // Jour de l'année (avec fraction)

    // === Éléments orbitaux (Ligne 2) ===
    double inclination;        // Inclinaison (degrés) [0-180°]
    double raan;              // Right Ascension of Ascending Node (degrés)
    double eccentricity;      // Excentricité [0-1[
    double argOfPerigee;      // Argument du périgée (degrés)
    double meanAnomaly;       // Anomalie moyenne (degrés) [0-360°]
    double meanMotion;        // Mouvement moyen (révolutions/jour)

    // === Paramètres de perturbation (Ligne 1) ===
    double bstar;             // Coefficient de traînée atmosphérique
    double meanMotionDot;     // 1ère dérivée du mouvement moyen
    double meanMotionDotDot;  // 2ème dérivée du mouvement moyen

    // === Métadonnées ===
    int revolutionNumber;      // Numéro de révolution à l'époque
    int elementSetNumber;      // Numéro du jeu d'éléments

    // === Paramètres calculés ===
    double period;            // Période orbitale (minutes)
    double semiMajorAxis;     // Demi-grand axe (km)
    double altitude;          // Altitude moyenne (km)

    /**
     * @brief Calcule les paramètres dérivés
     */
    void calculateDerivedParameters();
};

/**
 * @brief Parser pour les données TLE
 *
 * Parse les fichiers TLE à 2 ou 3 lignes et extrait
 * tous les éléments orbitaux nécessaires
 */
class TLEParser
{
public:
    /**
     * @brief Parse un TLE à partir de 3 lignes
     * @param line0 Ligne 0 : nom du satellite
     * @param line1 Ligne 1 : paramètres généraux
     * @param line2 Ligne 2 : éléments orbitaux
     * @return Structure TLEData complète
     * @throws QString si le format est invalide
     */
    static TLEData parseTLE(const QString& line0,
                            const QString& line1,
                            const QString& line2);

    /**
     * @brief Parse un TLE à partir de 2 lignes (sans nom)
     * @param line1 Ligne 1 : paramètres généraux
     * @param line2 Ligne 2 : éléments orbitaux
     * @return Structure TLEData (name sera vide)
     */
    static TLEData parseTLE(const QString& line1, const QString& line2);

    /**
     * @brief Vérifie la validité du checksum TLE
     * @param line Ligne à vérifier
     * @return true si le checksum est valide
     */
    static bool verifyChecksum(const QString& line);

    /**
     * @brief Extrait une valeur double d'une ligne TLE
     * @param line Ligne TLE
     * @param start Position de début
     * @param length Longueur du champ
     * @return Valeur extraite
     */
    static double extractDouble(const QString& line, int start, int length);

    /**
     * @brief Extrait une valeur entière d'une ligne TLE
     * @param line Ligne TLE
     * @param start Position de début
     * @param length Longueur du champ
     * @return Valeur extraite
     */
    static int extractInt(const QString& line, int start, int length);

    /**
     * @brief Convertit l'époque TLE en QDateTime
     * @param year Année (2 chiffres, format YY)
     * @param dayOfYear Jour de l'année avec fraction
     * @return QDateTime correspondant
     */
    static QDateTime epochToDateTime(int year, double dayOfYear);

private:
    /**
     * @brief Parse la notation scientifique compacte du TLE
     * Exemple : "12345-3" = 0.12345 × 10^-3
     */
    static double parseScientific(const QString& str);
};

#endif // TLEPARSER_H
