import QtQuick
import QtQuick.Controls
import QtQuick3D

Window {
    visible: true
    width: 1200
    height: 800
    title: "OrbiFrance 3D"

    property double simTime: 0

    View3D {
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "black"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 1000)
            eulerRotation: Qt.vector3d(0, 0, 0)
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            brightness: 1.0
        }

        // Terre
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(3, 3, 3)
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    source: "qrc:/res/textures/earth.jpg"
                }
            }
        }
/*
        // Soleil
        Model {
            source: "#Sphere"
            position: Qt.vector3d(10, 0, 0)
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            materials: DefaultMaterial {
                emissiveFactor: Qt.vector3d(1, 1, 0.8)
            }
        }
*/
        // Satellite
        Model {
            id: satellite
            source: "#Sphere"
            scale: Qt.vector3d(0.2, 0.2, 0.2)
            materials: DefaultMaterial { diffuseColor: "red" }

            property var pos: orbitCalculator.getSatellitePosition(simTime)
            position: pos
        }
    }

    // Slider temporel
    Slider {
        id: timeSlider
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: 20
        }
        from: 0
        to: 100
        onValueChanged: simTime = value
    }
}
