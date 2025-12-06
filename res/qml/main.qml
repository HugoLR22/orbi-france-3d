import QtQuick
import QtQuick.Controls
import QtQuick3D

Window {
    visible: true
    width: 1200
    height: 800
    title: "OrbiFrance 3D"

    property double simTime: 0

    // === GESTION DU TEMPS ===
    property date baseTime: new Date()  // Temps de référence (maintenant)
    property double timeOffset: 0        // Offset en secondes depuis baseTime
    property double timeScale: 60.0       // Échelle de temps (1 = temps réel, 60 = 1 min/sec)
    property bool timeRunning: true     // Animation temporelle active

    // Timer pour l'animation temporelle
    Timer {
        id: timeAnimationTimer
        interval: 16  //60 FPS
        repeat: true
        running: timeRunning
        onTriggered: {
            // Avancer le temps selon l'échelle
            timeOffset += (interval / 1000.0) * timeScale
        }
    }

    // Temps simulé actuel
    property date currentSimulatedTime: new Date(baseTime.getTime() + timeOffset * 1000)

    // === PROPRIÉTÉS DE CONTRÔLE CAMÉRA ===
    property real cameraDistance: 1000
    property real cameraRotationX: -20
    property real cameraRotationY: 0
    property real cameraPanX: 0
    property real cameraPanY: 0

    // === OPTIONS D'AFFICHAGE ORBITE ===
    property bool showOrbits: true

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
        // SATELLITES RÉELS
        // ========================================
        SatelliteRenderer {
            id: satelliteRenderer
            currentTime: currentSimulatedTime  // Utilise le temps simulé
            autoUpdate: true
            updateInterval: 100  // Mise à jour plus fréquente (10 FPS)
        }

        // ========================================
        // ORBITES DES SATELLITES
        // ========================================
        Node {
            id: orbitRendererNode
            visible: showOrbits

            property int orbitResolution: 64
            property var orbitObjects: ({})

            Component {
                id: orbitPointComponent

                Model {
                    source: "#Sphere"
                    scale: Qt.vector3d(0.05, 0.05, 0.05)

                    property color pointColor: "#00ff88"

                    materials: PrincipledMaterial {
                        baseColor: pointColor
                        lighting: PrincipledMaterial.NoLighting
                        emissiveFactor: Qt.vector3d(1.0, 1.0, 1.0)
                        opacity: 0.6
                        alphaMode: PrincipledMaterial.Blend
                    }
                }
            }

            function calculateOrbitPoints(satellite) {
                var positions = []
                var period = satellite.period * 60
                var referenceTime = new Date()

                for (var i = 0; i <= orbitResolution; i++) {
                    var timeOffset = (period * i) / orbitResolution
                    var pointTime = new Date(referenceTime.getTime() + timeOffset * 1000)
                    var satPositions = satelliteDatabase.calculateAllPositions(pointTime)

                    for (var j = 0; j < satPositions.length; j++) {
                        if (satPositions[j].id === satellite.id) {
                            positions.push({
                                x: satPositions[j].x,
                                y: satPositions[j].y,
                                z: satPositions[j].z
                            })
                            break
                        }
                    }
                }

                return positions
            }

            function createOrbitForSatellite(sat) {
                var points = calculateOrbitPoints(sat)

                if (points.length === 0) {
                    console.warn("  ⚠️ Aucun point pour", sat.name)
                    return
                }

                if (!orbitObjects[sat.id]) {
                    orbitObjects[sat.id] = []
                }

                // Nettoyer l'ancienne orbite
                for (var i = 0; i < orbitObjects[sat.id].length; i++) {
                    if (orbitObjects[sat.id][i]) {
                        orbitObjects[sat.id][i].destroy()
                    }
                }
                orbitObjects[sat.id] = []

                // Créer les points
                for (var j = 0; j < points.length; j++) {
                    var pt = points[j]

                    var sphere = orbitPointComponent.createObject(orbitRendererNode, {
                        "position": Qt.vector3d(pt.x, pt.y, pt.z),
                        "pointColor": sat.color
                    })

                    if (sphere) {
                        orbitObjects[sat.id].push(sphere)
                    }
                }

                console.log("  ✅", sat.name, ":", orbitObjects[sat.id].length, "points")
            }

            function updateOrbits() {
                console.log("🔄 Génération des orbites...")
                var satellites = satelliteDatabase.getAllSatellites()

                for (var i = 0; i < satellites.length; i++) {
                    createOrbitForSatellite(satellites[i])
                }

                console.log("✅", satellites.length, "orbites générées")
            }

            function reloadOrbits() {
                updateOrbits()
            }

            Component.onCompleted: {
                console.log("🛰️ OrbitRenderer initialisé")
                Qt.callLater(updateOrbits)
            }
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
        height: 150
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
                    width: parent.width - 260
                    from: -86400  // -24h
                    to: 86400     // +24h
                    value: timeOffset
                    onMoved: {
                        timeOffset = value
                    }
                }

                Text {
                    text: (timeOffset / 3600).toFixed(1) + " h"
                    color: "#00ff88"
                    width: 80
                    verticalAlignment: Text.AlignVCenter
                    height: timeSlider.height
                }

                Button {
                    text: "⏮️"
                    width: 40
                    onClicked: timeOffset = 0
                    ToolTip.visible: hovered
                    ToolTip.text: "Revenir au temps actuel"
                }
            }

            // Contrôle de la vitesse de simulation
            Row {
                width: parent.width
                spacing: 10

                Text {
                    text: "Vitesse:"
                    color: "white"
                    width: 80
                    verticalAlignment: Text.AlignVCenter
                    height: speedSlider.height
                }

                Slider {
                    id: speedSlider
                    width: parent.width - 180
                    from: 0
                    to: 200
                    value: timeScale
                    onMoved: timeScale = value
                }

                Text {
                    text: timeScale.toFixed(0) + "x"
                    color: "#00ff88"
                    width: 80
                    verticalAlignment: Text.AlignVCenter
                    height: speedSlider.height
                }
            }

            // Boutons de contrôle essentiels
            Row {
                spacing: 10

                Button {
                    text: timeRunning ? "⏸️ Pause" : "▶️ Play"
                    onClicked: timeRunning = !timeRunning
                    ToolTip.visible: hovered
                    ToolTip.text: "Pause/Reprendre la simulation temporelle"
                }

                Button {
                    text: "📷 Reset Vue"
                    onClicked: {
                        cameraDistance = 1000
                        cameraRotationX = -20
                        cameraRotationY = 0
                        cameraPanX = 0
                        cameraPanY = 0
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: "Réinitialise la position de la caméra"
                }

                Button {
                    text: showOrbits ? "🌐 Masquer Orbites" : "🌐 Afficher Orbites"
                    onClicked: showOrbits = !showOrbits
                    ToolTip.visible: hovered
                    ToolTip.text: "Affiche/masque les trajectoires orbitales"
                }

                // Séparateur visuel
                Rectangle {
                    width: 2
                    height: parent.height
                    color: "#333333"
                }

                Text {
                    text: "🛰️ " + satelliteDatabase.count + " satellites"
                    color: "#00ff88"
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                    leftPadding: 10
                    font.bold: true
                    font.pixelSize: 13
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
                text: "✋ Clic droit : Déplacement (Pan)"
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

    // Informations satellites détaillées
    Rectangle {
        anchors {
            top: parent.top
            left: parent.left
            margins: 10
        }
        width: 320
        height: satelliteListColumn.height + 20
        color: "#cc000000"
        radius: 5

        Column {
            id: satelliteListColumn
            anchors.centerIn: parent
            width: parent.width - 20
            spacing: 5

            // En-tête
            Row {
                width: parent.width
                spacing: 10

                Text {
                    text: "🛰️ SATELLITES FRANÇAIS (" + satelliteDatabase.count + ")"
                    color: "#00ff88"
                    font.bold: true
                    font.pixelSize: 13
                }

                Button {
                    text: showDetailedInfo ? "▼" : "▶"
                    width: 30
                    height: 20
                    onClicked: showDetailedInfo = !showDetailedInfo
                }
            }

            // Liste des satellites
            Repeater {
                model: satelliteDatabase.calculateAllPositions(currentSimulatedTime)

                delegate: Column {
                    width: satelliteListColumn.width
                    spacing: 2

                    Rectangle {
                        width: parent.width
                        height: 24
                        color: satelliteMouseArea.containsMouse ? "#22ffffff" : "transparent"
                        radius: 3

                        MouseArea {
                            id: satelliteMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                selectedSatelliteId = (selectedSatelliteId === modelData.id) ? "" : modelData.id
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 5
                            spacing: 8

                            Rectangle {
                                width: 14
                                height: 14
                                radius: 7
                                color: modelData.color
                                anchors.verticalCenter: parent.verticalCenter

                                SequentialAnimation on opacity {
                                    running: true
                                    loops: Animation.Infinite
                                    NumberAnimation { to: 0.6; duration: 1000 }
                                    NumberAnimation { to: 1.0; duration: 1000 }
                                }
                            }

                            Text {
                                text: modelData.name
                                color: "white"
                                font.pixelSize: 11
                                font.bold: selectedSatelliteId === modelData.id
                                width: 140
                                elide: Text.ElideRight
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: modelData.category
                                color: "#888888"
                                font.pixelSize: 9
                                width: 80
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: selectedSatelliteId === modelData.id ? "▼" : "▶"
                                color: "#00ff88"
                                font.pixelSize: 10
                                anchors.verticalCenter: parent.verticalCenter
                                visible: showDetailedInfo
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: detailsColumn.height + 10
                        color: "#11ffffff"
                        radius: 3
                        visible: showDetailedInfo && selectedSatelliteId === modelData.id

                        Column {
                            id: detailsColumn
                            anchors.centerIn: parent
                            width: parent.width - 10
                            spacing: 5

                            Row {
                                spacing: 5
                                Text {
                                    text: "🌍 Altitude:"
                                    color: "#00ff88"
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                                Text {
                                    text: modelData.altitude.toFixed(1) + " km"
                                    color: "white"
                                    font.pixelSize: 10
                                }
                            }

                            Row {
                                spacing: 5
                                Text {
                                    text: "🚀 Vitesse:"
                                    color: "#00ff88"
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                                Text {
                                    text: modelData.speed.toFixed(3) + " km/s"
                                    color: "white"
                                    font.pixelSize: 10
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: "#333333"
                    }
                }
            }

            // Affichage du temps simulé
            Rectangle {
                width: parent.width
                height: 25
                color: "#11ffffff"
                radius: 3

                Text {
                    anchors.centerIn: parent
                    text: "⏱️ " + currentSimulatedTime.toLocaleString(Qt.locale(), "dd/MM/yyyy HH:mm:ss")
                    color: "#00ff88"
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }
    }

    property string selectedSatelliteId: ""
    property bool showDetailedInfo: true
}
