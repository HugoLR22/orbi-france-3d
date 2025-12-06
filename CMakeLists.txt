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

# ============================================
# BIBLIOTHÈQUE EXTERNE : libsgp4
# ============================================

set(SGP4_DIR "${CMAKE_SOURCE_DIR}/external/sgp4/libsgp4")

if(EXISTS ${SGP4_DIR})
    message(STATUS "✅ libsgp4 trouvée: ${SGP4_DIR}")

    # Fichiers sources de libsgp4 (format .cc)
    file(GLOB SGP4_SOURCES "${SGP4_DIR}/*.cc")
    file(GLOB SGP4_HEADERS "${SGP4_DIR}/*.h")

    # Afficher les fichiers trouvés
    list(LENGTH SGP4_SOURCES SGP4_SOURCE_COUNT)
    message(STATUS "   ${SGP4_SOURCE_COUNT} fichiers .cc trouvés")

    # Ajouter le répertoire d'includes
    include_directories(${SGP4_DIR})

else()
    message(FATAL_ERROR "   libsgp4 non trouvée dans ${SGP4_DIR}")
    message(FATAL_ERROR "   Vérifiez que le dossier external/sgp4/libsgp4 existe")
    message(FATAL_ERROR "   Clonez la bibliothèque si nécessaire:")
    message(FATAL_ERROR "   cd external && git clone https://github.com/dnwrnr/sgp4.git")
endif()

# ============================================
# FICHIERS SOURCES - Organisation par module
# ============================================

set(SOURCES
    # Main
    src/main.cpp

    # Module Orbit (calculs orbitaux)
    src/orbit/OrbitCalculator.cpp
    src/orbit/OrbitPath.cpp

    # Module Data (gestion données satellites)
    src/data/TLEParser.cpp
    src/data/SGP4Propagator.cpp
    src/data/SatelliteDatabase.cpp

    # Bibliothèque externe SGP4
    ${SGP4_SOURCES}
)

set(HEADERS
    # Module Orbit
    src/orbit/OrbitCalculator.h
    src/orbit/OrbitPath.h

    # Module Data
    src/data/TLEParser.h
    src/data/SGP4Propagator.h
    src/data/SatelliteDatabase.h

    # Bibliothèque externe SGP4
    ${SGP4_HEADERS}
)

# Fichier de ressources Qt
qt_add_resources(RESOURCES resources.qrc)

# ============================================
# CRÉATION DE L'EXÉCUTABLE
# ============================================

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# ============================================
# LIENS AVEC LES BIBLIOTHÈQUES Qt
# ============================================

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Quick3D
)

# ============================================
# COPIE DES DONNÉES
# ============================================

# Copie le dossier data/ dans le dossier de build pour accès à l'exécution
set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
if(EXISTS ${DATA_DIR})
    file(COPY ${DATA_DIR} DESTINATION ${CMAKE_BINARY_DIR})
    message(STATUS "✅ Dossier data/ copié vers ${CMAKE_BINARY_DIR}/data")
else()
    message(WARNING "⚠️ Dossier data/ non trouvé à ${DATA_DIR}")
endif()

# ============================================
# PROPRIÉTÉS DU PROJET
# ============================================

set_target_properties(${PROJECT_NAME} PROPERTIES
    QT_QML_DEBUG TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ============================================
# AFFICHAGE DES INFORMATIONS DE BUILD
# ============================================

message(STATUS "")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS " ️  Configuration OrbiFrance 3D")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version:     ${Qt6_VERSION}")
message(STATUS "Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Dir:     ${CMAKE_SOURCE_DIR}")
message(STATUS "Build Dir:      ${CMAKE_BINARY_DIR}")
message(STATUS "")
message(STATUS "  Modules:")
message(STATUS "  - Orbit: OrbitCalculator, OrbitPath")
message(STATUS "  - Data:  TLEParser, SGP4Propagator")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "")
