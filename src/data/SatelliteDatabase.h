#ifndef SATELLITEDATABASE_H
#define SATELLITEDATABASE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QDateTime>
#include <QVariantList>
#include <QVariantMap>

#include "TLEParser.h"
#include "SGP4Propagator.h"

/**
 * @brief Structure représentant un satellite avec ses données
 */
struct SatelliteData {
    QString id;                  // Identifiant unique (ex: "SPOT7")
    QString name;                // Nom complet
    int noradId;                 // Numéro NORAD
    QString category;            // Catégorie: "Observation", "Telecom", "Navigation", "Militaire"
    QString mission;             // Description de la mission
    QDateTime launchDate;        // Date de lancement
    bool isActive;              // Statut actif/inactif
    
    TLEData tle;                // Données TLE
    SGP4Propagator* propagator; // Propagateur SGP4
    
    // Propriétés visuelles
    QString color;              // Couleur d'affichage (hex)
    double displayScale;        // Échelle d'affichage (défaut: 1.0)
    bool visible;               // Visible dans la scène 3D
    
    // Constructeur
    SatelliteData() : noradId(0), isActive(true), propagator(nullptr), 
                     displayScale(1.0), visible(true) {}
    
    ~SatelliteData() {
        delete propagator;
    }
    
    /**
     * @brief Convertit en QVariantMap pour QML
     */
    QVariantMap toVariantMap() const;
};

/**
 * @brief Base de données de satellites français
 * 
 * Gère une collection de satellites avec leurs TLE et propagateurs.
 * Permet de filtrer, rechercher et mettre à jour les satellites.
 */
class SatelliteDatabase : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY satellitesChanged)
    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
    
public:
    explicit SatelliteDatabase(QObject *parent = nullptr);
    ~SatelliteDatabase();
    
    /**
     * @brief Ajoute un satellite à partir d'un TLE
     * @param id Identifiant unique
     * @param category Catégorie du satellite
     * @param color Couleur d'affichage (hex, ex: "#ff0000")
     * @param tle Données TLE
     * @return true si ajouté avec succès
     */
    bool addSatellite(const QString& id, 
                     const QString& category,
                     const QString& color,
                     const TLEData& tle);
    
    /**
     * @brief Charge des satellites prédéfinis (satellites français)
     */
    Q_INVOKABLE void loadFrenchSatellites();
    
    /**
     * @brief Charge des satellites depuis des lignes TLE
     * @param tleLines Liste de lignes TLE (format : ligne0, ligne1, ligne2, ligne0, ligne1, ligne2, ...)
     * @param category Catégorie pour tous les satellites
     * @param color Couleur par défaut
     */
    Q_INVOKABLE void loadFromTLELines(const QStringList& tleLines, 
                                       const QString& category = "Custom",
                                       const QString& color = "#00ff00");
    
    /**
     * @brief Récupère tous les satellites
     * @return Liste de QVariantMap pour QML
     */
    Q_INVOKABLE QVariantList getAllSatellites() const;
    
    /**
     * @brief Récupère les satellites d'une catégorie
     */
    Q_INVOKABLE QVariantList getSatellitesByCategory(const QString& category) const;
    
    /**
     * @brief Recherche un satellite par ID
     */
    Q_INVOKABLE QVariantMap getSatelliteById(const QString& id) const;
    
    /**
     * @brief Recherche un satellite par NORAD ID
     */
    Q_INVOKABLE QVariantMap getSatelliteByNoradId(int noradId) const;
    
    /**
     * @brief Calcule les positions de tous les satellites à un instant donné
     * @param dateTime Date/heure (UTC)
     * @return Liste de positions {id, name, position, velocity, visible}
     */
    Q_INVOKABLE QVariantList calculateAllPositions(const QDateTime& dateTime) const;
    
    /**
     * @brief Active/désactive la visibilité d'un satellite
     */
    Q_INVOKABLE void setSatelliteVisible(const QString& id, bool visible);
    
    /**
     * @brief Active/désactive la visibilité d'une catégorie
     */
    Q_INVOKABLE void setCategoryVisible(const QString& category, bool visible);
    
    /**
     * @brief Supprime un satellite
     */
    Q_INVOKABLE bool removeSatellite(const QString& id);
    
    /**
     * @brief Vide la base de données
     */
    Q_INVOKABLE void clear();
    
    // Getters
    int count() const { return m_satellites.size(); }
    QStringList categories() const;
    
signals:
    void satellitesChanged();
    void categoriesChanged();
    void satelliteAdded(const QString& id);
    void satelliteRemoved(const QString& id);
    
private:
    QVector<SatelliteData*> m_satellites;
    
    /**
     * @brief Trouve un satellite par ID
     * @return Pointeur vers le satellite ou nullptr
     */
    SatelliteData* findSatelliteById(const QString& id) const;
    
    /**
     * @brief Génère une couleur par défaut selon la catégorie
     */
    QString getDefaultColor(const QString& category) const;
};

#endif // SATELLITEDATABASE_H