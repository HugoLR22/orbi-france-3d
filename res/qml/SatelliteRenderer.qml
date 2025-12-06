import QtQuick
import QtQuick3D

/**
 * Composant pour afficher tous les satellites de la base de données
 */
Node {
    id: satelliteRenderer

    property var currentTime: new Date()
    property bool autoUpdate: true
    property int updateInterval: 1000  // Mise à jour toutes les secondes

    // Timer pour mise à jour automatique
    Timer {
        running: autoUpdate
        interval: updateInterval
        repeat: true
        onTriggered: updateSatellites()
    }

    // Composant pour un satellite individuel
    Component {
        id: satelliteComponent

        Node {
            id: satelliteNode

            property string satId: ""
            property string satName: ""
            property string satCategory: ""
            property color satColor: "#ffffff"

            // Modèle 3D du satellite
            Model {
                source: "#Sphere"
                scale: Qt.vector3d(0.06, 0.06, 0.06)

                materials: PrincipledMaterial {
                    baseColor: satColor
                    metalness: 0.8
                    roughness: 0.3
                    emissiveFactor: Qt.vector3d(0.8, 0.8, 0.8)
                    lighting: PrincipledMaterial.NoLighting
                }
            }

            // Label du satellite (optionnel - commenté pour performances)
            /*
            Model {
                source: "#Cube"
                scale: Qt.vector3d(0.5, 0.1, 0.05)
                position: Qt.vector3d(0, 0.3, 0)

                materials: PrincipledMaterial {
                    baseColor: satColor
                    opacity: 0.8
                    alphaMode: PrincipledMaterial.Blend
                }
            }
            */
        }
    }

    // Liste des objets satellites créés
    property var satelliteObjects: ({})

    /**
     * Crée ou met à jour l'affichage de tous les satellites
     */
    function updateSatellites() {
        // Calculer les positions à l'instant actuel
        var positions = satelliteDatabase.calculateAllPositions(currentTime)

        for (var i = 0; i < positions.length; i++) {
            var satData = positions[i]
            var satId = satData.id

            // Si le satellite n'existe pas encore, le créer
            if (!satelliteObjects[satId]) {
                var satObj = satelliteComponent.createObject(satelliteRenderer, {
                    "satId": satId,
                    "satName": satData.name,
                    "satCategory": satData.category,
                    "satColor": satData.color
                })

                if (satObj) {
                    satelliteObjects[satId] = satObj
                    console.log("✅ Satellite créé:", satData.name)
                } else {
                    console.error("❌ Échec création satellite:", satData.name)
                    continue
                }
            }

            // Mettre à jour la position
            var satNode = satelliteObjects[satId]
            if (satNode) {
                satNode.position = Qt.vector3d(satData.x, satData.y, satData.z)
            }
        }
    }

    /**
     * Supprime tous les satellites
     */
    function clearSatellites() {
        for (var satId in satelliteObjects) {
            if (satelliteObjects[satId]) {
                satelliteObjects[satId].destroy()
            }
        }
        satelliteObjects = {}
        console.log("🗑️ Tous les satellites supprimés")
    }

    /**
     * Recharge tous les satellites
     */
    function reloadSatellites() {
        clearSatellites()
        updateSatellites()
        console.log("🔄 Satellites rechargés")
    }

    // Initialisation au démarrage
    Component.onCompleted: {
        console.log("🛰️ SatelliteRenderer initialisé")
        updateSatellites()
    }

    // Nettoyage à la destruction
    Component.onDestruction: {
        clearSatellites()
    }
}
