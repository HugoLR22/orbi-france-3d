import QtQuick
import QtQuick.Controls
import QtQuick3D

Window {
    visible: true
    width: 1200
    height: 800
    title: "OrbiFrance 3D"

    property double simTime: 0

    // === PROPRIÉTÉS DE CONTRÔLE CAMÉRA ===
    property real cameraDistance: 1000
    property real cameraRotationX: -20
    property real cameraRotationY: 0
    property real cameraPanX: 0
    property real cameraPanY: 0

    // === OPTIONS D'AFFICHAGE ORBITE ===
    property bool showOrbitLine: true

    // États de la souris
    property bool isDragging: false
    property bool isPanning: false
    property point lastMousePos: Qt.point(0, 0)

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#000000"
            backgroundMode: SceneEnvironment.Color
            // Active l'antialiasing pour un rendu plus lisse
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        // === CAMÉRA AVEC CONTRÔLES INTERACTIFS ===
        Node {
            id: cameraRoot
            position: Qt.vector3d(cameraPanX, cameraPanY, 0)

            Node {
                id: cameraRotator
                eulerRotation: Qt.vector3d(cameraRotationX, cameraRotationY, 0)

                PerspectiveCamera {
                    id: camera
                    position: Qt.vector3d(0, 0, cameraDistance)
                    fieldOfView: 45
                    clipNear: 1
                    clipFar: 20000
                }
            }
        }

        // ========================================
        // FOND ÉTOILÉ - DOIT ÊTRE LE PREMIER OBJET
        // ========================================

        Model {
            id: testStars
            source: "#Sphere"
            position: Qt.vector3d(0, 0, 0)  // Devant la Terre
            scale: Qt.vector3d(100, 100, 100)

            materials: DefaultMaterial {
                diffuseMap: Texture {
                    id: starsTexture
                    source: "qrc:/res/textures/stars.jpg"
                }
                lighting: DefaultMaterial.NoLighting
                cullMode: Material.NoCulling
            }
        }

        // Lumière principale (Soleil)
        DirectionalLight {
            id: sunLight
            eulerRotation.x: -45
            eulerRotation.y: 45
            brightness: 1.5
            castsShadow: false  // Désactivé pour éviter les artefacts
        }

        // Lumière ambiante faible pour éviter que la face cachée soit totalement noire
        DirectionalLight {
            eulerRotation.x: 135
            eulerRotation.y: -135
            brightness: 0.15
            color: "#1a3a52"  // Teinte bleutée subtile
        }

        // ========================================
        // TERRE - Sphère principale
        // ========================================
        Model {
            id: earth
            source: "#Sphere"
            scale: Qt.vector3d(3, 3, 3)

            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "qrc:/res/textures/earth-day.jpg"
                    generateMipmaps: true
                    mipFilter: Texture.Linear
                }
                // Propriétés pour un aspect plus réaliste
                metalness: 0.0
                roughness: 0.9
            }

            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 30000
                loops: Animation.Infinite
            }
        }

        // ========================================
        // NUAGES - Sphère atmosphérique
        // ========================================
        Model {
            id: clouds
            source: "#Sphere"
            // Légèrement plus grande pour éviter le z-fighting
            scale: Qt.vector3d(3.06, 3.06, 3.06)

            materials: PrincipledMaterial {
                baseColorMap: Texture {
                    source: "qrc:/res/textures/earth-clouds.jpg"
                    generateMipmaps: true
                    mipFilter: Texture.Linear
                }
                // Configuration de transparence
                alphaMode: PrincipledMaterial.Blend
                opacity: 0.6

                // Pas d'éclairage direct pour effet translucide
                lighting: PrincipledMaterial.NoLighting

                // Légère émission pour les nuages éclairés
                emissiveFactor: Qt.vector3d(0.4, 0.4, 0.4)

                // Propriétés physiques
                metalness: 0.0
                roughness: 1.0

                // Culling désactivé pour voir les deux faces
                cullMode: Material.NoCulling
            }

            // Rotation légèrement plus lente pour effet de dérive
            // (les nuages ne suivent pas exactement la rotation terrestre)
            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 32000  // 2s de plus que la Terre
                loops: Animation.Infinite
            }
        }

        // ========================================
        // TRAJECTOIRE ORBITALE - LIGNE SIMULÉE
        // ========================================
        Node {
            id: orbitContainer
            visible: showOrbitLine

            property var orbitPoints: []

            Component {
                id: orbitPointComponent

                Model {
                    source: "#Sphere"
                    scale: Qt.vector3d(0.015, 0.015, 0.015)  // Très petites sphères

                    materials: PrincipledMaterial {
                        lighting: PrincipledMaterial.NoLighting
                        emissiveFactor: Qt.vector3d(1.5, 1.5, 1.5)
                    }
                }
            }

            Component.onCompleted: {
                var points = orbitPath.generateOrbitPoints()
                console.log("🛰️ Génération orbite:", points.length, "points")

                if (points.length < 2) {
                    console.warn("Pas assez de points")
                    return
                }

                console.log("📍 Premier point:", points[0].x, points[0].y, points[0].z)
                orbitPoints = points

                // Créer une sphère pour chaque point
                var createdCount = 0
                for (var i = 0; i < points.length; i++) {
                    var pt = points[i]

                    var sphere = orbitPointComponent.createObject(orbitContainer, {
                        "position": Qt.vector3d(pt.x, pt.y, pt.z)
                    })

                    if (sphere !== null) {
                        createdCount++
                    }
                }

                console.log("✅ Orbite créée:", createdCount, "points sur", points.length)
            }
        }

        // ========================================
        // SATELLITE
        // ========================================
        Model {
            id: satellite
            source: "#Sphere"
            scale: Qt.vector3d(0.2, 0.2, 0.2)

            materials: PrincipledMaterial {
                baseColor: "#ff3333"
                metalness: 0.8
                roughness: 0.3
                // Légère émission pour visibilité dans l'ombre
                emissiveFactor: Qt.vector3d(0.2, 0.05, 0.05)
            }

            property var pos: orbitCalculator.getSatellitePosition(simTime)
            position: pos
        }
    }

    // ========================================
    // GESTION DES CONTRÔLES SOURIS
    // ========================================
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

        onWheel: (wheel) => {
            let delta = wheel.angleDelta.y / 120
            cameraDistance = Math.max(200, Math.min(5000, cameraDistance - delta * 50))
        }

        onPressed: (mouse) => {
            lastMousePos = Qt.point(mouse.x, mouse.y)

            if (mouse.button === Qt.LeftButton) {
                isDragging = true
            } else if (mouse.button === Qt.RightButton || mouse.button === Qt.MiddleButton) {
                isPanning = true
            }
        }

        onReleased: {
            isDragging = false
            isPanning = false
        }

        onPositionChanged: (mouse) => {
            if (!isDragging && !isPanning)
                return

            let deltaX = mouse.x - lastMousePos.x
            let deltaY = mouse.y - lastMousePos.y

            if (isDragging) {
                cameraRotationY += deltaX * 0.5
                cameraRotationX = Math.max(-89, Math.min(89, cameraRotationX - deltaY * 0.5))
            }

            if (isPanning) {
                let panSpeed = cameraDistance / 500
                cameraPanX += deltaX * panSpeed
                cameraPanY -= deltaY * panSpeed
            }

            lastMousePos = Qt.point(mouse.x, mouse.y)
        }
    }


    // ========================================
    // INTERFACE UTILISATEUR
    // ========================================

    // Panneau de contrôle
    Rectangle {
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: 120
        color: "#cc000000"

        Column {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10

            // Slider temporel
            Row {
                width: parent.width
                spacing: 10

                Text {
                    text: "Temps:"
                    color: "white"
                    width: 80
                    verticalAlignment: Text.AlignVCenter
                    height: timeSlider.height
                }

                Slider {
                    id: timeSlider
                    width: parent.width - 180
                    from: 0
                    to: 100
                    value: 0
                    onValueChanged: simTime = value
                }

                Text {
                    text: simTime.toFixed(1) + " s"
                    color: "white"
                    width: 80
                    verticalAlignment: Text.AlignVCenter
                    height: timeSlider.height
                }
            }

            // Boutons de contrôle
            Row {
                spacing: 10

                Button {
                    text: "📷 Réinitialiser Vue"
                    onClicked: {
                        cameraDistance = 1000
                        cameraRotationX = -20
                        cameraRotationY = 0
                        cameraPanX = 0
                        cameraPanY = 0
                    }
                }

                Button {
                    text: showOrbitLine ? "🔴 Masquer Orbite" : "🟢 Afficher Orbite"
                    onClicked: showOrbitLine = !showOrbitLine
                }

                Text {
                    text: "Distance: " + cameraDistance.toFixed(0)
                    color: "#888888"
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                    leftPadding: 20
                }
            }
        }
    }

    // Aide des contrôles
    Rectangle {
        anchors {
            top: parent.top
            right: parent.right
            margins: 10
        }
        width: 240
        height: helpColumn.height + 20
        color: "#cc000000"
        radius: 5

        Column {
            id: helpColumn
            anchors.centerIn: parent
            spacing: 5

            Text {
                text: "🖱️ CONTRÔLES CAMÉRA"
                color: "#00ff88"
                font.bold: true
                font.pixelSize: 14
            }
            Text {
                text: "🔄 Clic gauche : Rotation"
                color: "white"
                font.pixelSize: 12
            }
            Text {
                text: "✋ Clic droit : Déplacement"
                color: "white"
                font.pixelSize: 12
            }
            Text {
                text: "🔍 Molette : Zoom in/out"
                color: "white"
                font.pixelSize: 12
            }
        }
    }

    // Informations de debug (coin supérieur gauche)
    Rectangle {
        anchors {
            top: parent.top
            left: parent.left
            margins: 10
        }
        width: 200
        height: debugColumn.height + 20
        color: "#cc000000"
        radius: 5

        Column {
            id: debugColumn
            anchors.centerIn: parent
            spacing: 3

            Text {
                text: "🛰️ SATELLITE"
                color: "#ff3333"
                font.bold: true
                font.pixelSize: 12
            }
            Text {
                text: "Position X: " + satellite.pos.x.toFixed(1)
                color: "white"
                font.pixelSize: 10
            }
            Text {
                text: "Position Y: " + satellite.pos.y.toFixed(1)
                color: "white"
                font.pixelSize: 10
            }
            Text {
                text: "Position Z: " + satellite.pos.z.toFixed(1)
                color: "white"
                font.pixelSize: 10
            }
        }
    }
}

