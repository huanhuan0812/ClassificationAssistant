cmake_minimum_required(VERSION 3.16)

project(ClassificationAssistant VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Network)

qt_standard_project_setup()

qt_add_executable(ClassificationAssistant
    main.cpp
    settings.h
    settings.cpp
    settingsdialog.h
    settingsdialog.cpp
    classificationworker.h
    classificationworker.cpp
    mainwindow.cpp
    mainwindow.h
    stylesheet.h
)

target_link_libraries(ClassificationAssistant PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(ClassificationAssistant PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ClassificationAssistant
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

find_package(onnxruntime REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime::onnxruntime)
