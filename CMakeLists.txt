cmake_minimum_required(VERSION 3.16)

project(tsena VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql PrintSupport REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(tsena
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        databasemanager.h databasemanager.cpp
        custommessagebox.h custommessagebox.cpp
        app.h app.cpp app.ui
        ressource.qrc

        ajoutfournisseur.h ajoutfournisseur.cpp ajoutfournisseur.ui
        ajoutnouveauproduits.h ajoutnouveauproduits.cpp ajoutnouveauproduits.ui
        etatstock.h etatstock.cpp etatstock.ui
        mouvementdestock.h mouvementdestock.cpp mouvementdestock.ui
        ajoutclient.h ajoutclient.cpp ajoutclient.ui
        ajoutvente.h ajoutvente.cpp ajoutvente.ui
        financecompta.h financecompta.cpp financecompta.ui
        ../../Downloads/CartoBMW/img/annuler.png ../../Downloads/CartoBMW/img/faire-pivoter-le-symbole-dinterface-fleche-circulaire-gauche.png ../../Downloads/CartoBMW/img/filtrage-des-donnees.png ../../Downloads/CartoBMW/img/systeme-de-filtrage-des-donnees.png
        bondelivraison.h bondelivraison.cpp bondelivraison.ui
        operation.h operation.cpp operation.ui
        tiers.h tiers.cpp tiers.ui
        animatedhidableframe.h animatedhidableframe.cpp
        historique.h historique.cpp historique.ui
        retour.h retour.cpp retour.ui
        retourdialog.h retourdialog.cpp







    )
    # Ajoutez cette ligne après qt_add_executable
    target_include_directories(tsena PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tsena APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(tsena SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(tsena
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(tsena PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::PrintSupport)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.tsena)
endif()
set_target_properties(tsena PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS tsena
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(tsena)
endif()
