cmake_minimum_required(VERSION 3.16)

# Nom du projet
project(OrbiFrance3D VERSION 1.0 LANGUAGES CXX)

# Paramètres C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Active les fonctionnalités automatiques de Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Trouve les modules Qt nécessaires
find_package(Qt6 REQUIRED COMPONENTS Core Quick Quick3D)

# Fichiers sources
set(SOURCES
    src/main.cpp
    src/orbit/OrbitCalculator.cpp
)

set(HEADERS
    src/orbit/OrbitCalculator.h
)

# Fichier de ressources Qt
qt_add_resources(RESOURCES resources.qrc)

# Création de l'exécutable
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}      # inclut le .qrc compilé
)

# Lien avec les modules Qt nécessaires
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Quick3D
)

# Copie le dossier data/ dans le dossier de build pour accès à l'exécution
set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
file(COPY ${DATA_DIR} DESTINATION ${CMAKE_BINARY_DIR})

# Définir le répertoire de sortie et activer le debug QML
set_target_properties(${PROJECT_NAME} PROPERTIES
    QT_QML_DEBUG TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
