#include "SatelliteDatabase.h"
#include <QDebug>

QVariantMap SatelliteData::toVariantMap() const
{
    QVariantMap map;
    map["id"] = id;
    map["name"] = name;
    map["noradId"] = noradId;
    map["category"] = category;
    map["mission"] = mission;
    map["isActive"] = isActive;
    map["color"] = color;
    map["displayScale"] = displayScale;
    map["visible"] = visible;
    
    // Données orbitales
    map["altitude"] = tle.altitude;
    map["inclination"] = tle.inclination;
    map["period"] = tle.period;
    map["eccentricity"] = tle.eccentricity;
    
    return map;
}

SatelliteDatabase::SatelliteDatabase(QObject *parent)
    : QObject(parent)
{
}

SatelliteDatabase::~SatelliteDatabase()
{
    clear();
}

bool SatelliteDatabase::addSatellite(const QString& id, 
                                    const QString& category,
                                    const QString& color,
                                    const TLEData& tle)
{
    // Vérifier si le satellite existe déjà
    if (findSatelliteById(id) != nullptr) {
        qWarning() << "️ Satellite" << id << "existe déjà";
        return false;
    }
    
    // Créer le satellite
    SatelliteData* satellite = new SatelliteData();
    satellite->id = id;
    satellite->name = tle.name;
    satellite->noradId = tle.noradId;
    satellite->category = category;
    satellite->tle = tle;
    satellite->color = color.isEmpty() ? getDefaultColor(category) : color;
    satellite->isActive = true;
    satellite->visible = true;
    
    // Créer et initialiser le propagateur
    satellite->propagator = new SGP4Propagator(this);
    if (!satellite->propagator->initialize(tle)) {
        qWarning() << " Échec initialisation propagateur pour" << id;
        delete satellite;
        return false;
    }
    
    m_satellites.append(satellite);
    
    qDebug() << "✅ Satellite ajouté:" << id << "(" << category << ")";
    
    emit satellitesChanged();
    emit satelliteAdded(id);
    
    return true;
}

void SatelliteDatabase::loadFrenchSatellites()
{
    qDebug() << "";
    qDebug() << " Chargement des satellites français...";
    qDebug() << "";
    
    // === SATELLITES D'OBSERVATION ===
    
    // SPOT 7 (Observation Terre haute résolution)
    {
        QString line0 = "SPOT 7";
        QString line1 = "1 40053U 14034A   25312.49140299  .00000931  00000+0  20068-3 0  9996";
        QString line2 = "2 40053  98.0951  15.3494 0001252 103.7997 256.3343 14.60664452604912";
        TLEData tle = TLEParser::parseTLE(line0, line1, line2);
        addSatellite("SPOT7", "Observation", "#00ff88", tle);
    }
    
    // SPOT 6 (Jumeau de SPOT 7)
    {
        QString line0 = "SPOT 6";
        QString line1 = "1 38755U 12047A   25312.50646038  .00000666  00000+0  15303-3 0  9999";
        QString line2 = "2 38755  98.1615  17.9137 0001128  83.8077 276.3251 14.58552064700882";
        TLEData tle = TLEParser::parseTLE(line0, line1, line2);
        addSatellite("SPOT6", "Observation", "#00ff88", tle);
    }
    
    // Pléiades Neo 3 (Imagerie très haute résolution - 30 cm)
    {
        QString line0 = "PLEIADES NEO 3";
        QString line1 = "1 48268U 21034A   25312.48951185  .00000311  00000+0  46164-4 0  9991";
        QString line2 = "2 48268  97.8950  25.4218 0001282  98.0253 262.1106 14.81674354244977";
        TLEData tle = TLEParser::parseTLE(line0, line1, line2);
        addSatellite("PLEIADES_NEO3", "Observation", "#00ffff", tle);
    }
    
    // === SATELLITES DE TÉLÉCOMMUNICATIONS ===
    
    // Syracuse 4A (Télécommunications militaires)
    {
        QString line0 = "SYRACUSE 4A";
        QString line1 = "1 49333U 21095B   25311.54152477  .00000141  00000+0  00000+0 0  9990";
        QString line2 = "2 49333   0.0143 291.3016 0001512 288.6101  67.5492  1.00267411 15730";
        TLEData tle = TLEParser::parseTLE(line0, line1, line2);
        addSatellite("SYRACUSE4A", "Telecom", "#ff6600", tle);
    }
    
    // === SATELLITES DE NAVIGATION (Galileo français) ===
    
    // Galileo 11 (FOC FM7 - constellation européenne, contribution française)
    {
        QString line0 = "GSAT0208 (GALILEO 11)";
        QString line1 = "1 41175U 15079B   25310.35715630  .00000048  00000+0  00000+0 0  9991";
        QString line2 = "2 41175  55.6890 107.3763 0002145  23.4728 336.5246  1.70474029 61390";
        TLEData tle = TLEParser::parseTLE(line0, line1, line2);
        addSatellite("GALILEO11", "Navigation", "#ffff00", tle);
    }
    
    // === SATELLITES SCIENTIFIQUES ===
    
    // EXO-0 (Démonstrateur de mission d’observation d’exoplanètes)
    {
        QString line0 = "EXO-0";
        QString line1 = "1 59175U 23174DK  25312.43141375  .00014174  00000+0  49335-3 0  9998";
        QString line2 = "2 59175  97.4047  27.9103 0008848 230.5689 129.4769 15.29763468111230";
        TLEData tle = TLEParser::parseTLE(line0, line1, line2);
        addSatellite("EXO-0", "Scientifique", "#ff00ff", tle);
    }
    
    qDebug() << "";
    qDebug() << "" << m_satellites.size() << "satellites français chargés";
    qDebug() << "";
    
    emit categoriesChanged();
}

void SatelliteDatabase::loadFromTLELines(const QStringList& tleLines, 
                                        const QString& category,
                                        const QString& color)
{
    if (tleLines.size() % 3 != 0) {
        qWarning() << " Format TLE invalide : le nombre de lignes doit être un multiple de 3";
        return;
    }
    
    int loaded = 0;
    for (int i = 0; i < tleLines.size(); i += 3) {
        QString line0 = tleLines[i];
        QString line1 = tleLines[i + 1];
        QString line2 = tleLines[i + 2];
        
        try {
            TLEData tle = TLEParser::parseTLE(line0, line1, line2);
            QString id = QString("SAT_%1").arg(tle.noradId);
            
            if (addSatellite(id, category, color, tle)) {
                loaded++;
            }
        } catch (...) {
            qWarning() << "️ Erreur parsing TLE ligne" << i;
        }
    }
    
    qDebug() << "" << loaded << "satellites chargés depuis TLE";
}

QVariantList SatelliteDatabase::getAllSatellites() const
{
    QVariantList list;
    for (const SatelliteData* sat : m_satellites) {
        list.append(sat->toVariantMap());
    }
    return list;
}

QVariantList SatelliteDatabase::getSatellitesByCategory(const QString& category) const
{
    QVariantList list;
    for (const SatelliteData* sat : m_satellites) {
        if (sat->category == category) {
            list.append(sat->toVariantMap());
        }
    }
    return list;
}

QVariantMap SatelliteDatabase::getSatelliteById(const QString& id) const
{
    SatelliteData* sat = findSatelliteById(id);
    if (sat) {
        return sat->toVariantMap();
    }
    return QVariantMap();
}

QVariantMap SatelliteDatabase::getSatelliteByNoradId(int noradId) const
{
    for (const SatelliteData* sat : m_satellites) {
        if (sat->noradId == noradId) {
            return sat->toVariantMap();
        }
    }
    return QVariantMap();
}

QVariantList SatelliteDatabase::calculateAllPositions(const QDateTime& dateTime) const
{
    QVariantList positions;
    
    for (const SatelliteData* sat : m_satellites) {
        if (!sat->visible || !sat->propagator) {
            continue;
        }
        
        QVector3D position, velocity;
        if (sat->propagator->propagate(dateTime, position, velocity)) {
            QVariantMap satPos;
            satPos["id"] = sat->id;
            satPos["name"] = sat->name;
            satPos["category"] = sat->category;
            satPos["color"] = sat->color;
            
            // Position ECI convertie pour affichage
            QVector3D displayPos = SGP4Propagator::eciToDisplay(position, sat->displayScale);
            satPos["x"] = displayPos.x();
            satPos["y"] = displayPos.y();
            satPos["z"] = displayPos.z();
            
            // Vitesse
            satPos["vx"] = velocity.x();
            satPos["vy"] = velocity.y();
            satPos["vz"] = velocity.z();
            satPos["speed"] = velocity.length();
            
            // Altitude
            satPos["altitude"] = position.length() - 6371.0;
            
            positions.append(satPos);
        }
    }
    
    return positions;
}

void SatelliteDatabase::setSatelliteVisible(const QString& id, bool visible)
{
    SatelliteData* sat = findSatelliteById(id);
    if (sat) {
        sat->visible = visible;
        emit satellitesChanged();
    }
}

void SatelliteDatabase::setCategoryVisible(const QString& category, bool visible)
{
    int count = 0;
    for (SatelliteData* sat : m_satellites) {
        if (sat->category == category) {
            sat->visible = visible;
            count++;
        }
    }
    
    if (count > 0) {
        qDebug() << (visible ? " Affichage" : " Masquage") << "de" << count
                 << "satellites de catégorie" << category;
        emit satellitesChanged();
    }
}

bool SatelliteDatabase::removeSatellite(const QString& id)
{
    for (int i = 0; i < m_satellites.size(); i++) {
        if (m_satellites[i]->id == id) {
            delete m_satellites[i];
            m_satellites.remove(i);
            
            qDebug() << "🗑️ Satellite supprimé:" << id;
            
            emit satellitesChanged();
            emit satelliteRemoved(id);
            return true;
        }
    }
    return false;
}

void SatelliteDatabase::clear()
{
    qDeleteAll(m_satellites);
    m_satellites.clear();
    
    emit satellitesChanged();
    emit categoriesChanged();
}

QStringList SatelliteDatabase::categories() const
{
    QStringList cats;
    for (const SatelliteData* sat : m_satellites) {
        if (!cats.contains(sat->category)) {
            cats.append(sat->category);
        }
    }
    return cats;
}

SatelliteData* SatelliteDatabase::findSatelliteById(const QString& id) const
{
    for (SatelliteData* sat : m_satellites) {
        if (sat->id == id) {
            return sat;
        }
    }
    return nullptr;
}

QString SatelliteDatabase::getDefaultColor(const QString& category) const
{
    if (category == "Observation") return "#00ff88";
    if (category == "Telecom") return "#ff6600";
    if (category == "Navigation") return "#ffff00";
    if (category == "Scientifique") return "#ff00ff";
    if (category == "Militaire") return "#ff0000";
    return "#ffffff";  // Blanc par défaut
}
