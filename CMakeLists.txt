cmake_minimum_required(VERSION 3.16)

project(Q3270 VERSION 0.2.0 LANGUAGES CXX)

# append Git hash to build to track commits
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(Q3270_VERSION_FULL "${PROJECT_VERSION}+g${GIT_HASH}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/Version.h
    @ONLY
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Meta compilers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Default to Release if nothing specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Find Qt (Qt5 or Qt6)
find_package(Qt6 QUIET COMPONENTS Core Gui Widgets Network Svg SvgWidgets)
if(NOT Qt6_FOUND)
  find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network Svg)
endif()

add_compile_options(-Wall -Wextra -Wunused-function -fdiagnostics-show-option -fno-diagnostics-color)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQ3270_VERSION=\"${Q3270_VERSION_FULL}\"
)

set(SOURCES
    ActiveSettings.cpp
    Cell.cpp
    CertificateDetails.cpp
    CodePage.cpp
    ColourTheme.cpp
    DisplayScreen/ClickableSvgItem.cpp
    DisplayScreen/DisplayScreen.cpp
    DisplayScreen/DisplayScreen_Cursor.cpp
    DisplayScreen/DisplayScreen_Status.cpp
    DisplayScreen/DisplayScreen_Mouse.cpp
    DisplayScreen/LockIndicator.cpp
    FunctionRegistry.cpp
    Models/Colours.cpp
    Models/KeyboardMap.cpp
    Preferences/ColourSwatchWidget.cpp
    Preferences/KeyboardMapWidget.cpp
    Sessions/ManageAutoStartDialog.cpp
    Sessions/ManageSessionsDialog.cpp
    Sessions/OpenSessionDialog.cpp
    HostAddressUtils.cpp
    Keyboard.cpp
    KeyboardThemeDialog.cpp
    MainWindow.cpp
    Preferences/PreferencesDialog.cpp
    ProcessDataStream.cpp
    Sessions/SaveSessionDialog.cpp
    Models/Session.cpp
    Sessions/SessionDialogBase.cpp
    Sessions/SessionPreviewWidget.cpp
    Stores/ColourStore.cpp
    Stores/KeyboardStore.cpp
    Stores/SessionStore.cpp
    Terminal.cpp
    main.cpp
    SocketConnection.cpp
)

set(HEADERS
    ActiveSettings.h
    Cell.h
    CertificateDetails.h
    CodePage.h
    ColourTheme.h
    DisplayScreen.h
    DisplayScreen/ClickableSvgItem.h
    FunctionRegistry.h
    Models/Colours.h
    Preferences/ColourSwatchWidget.h
    Preferences/KeyboardMapWidget.h
    Sessions/ManageAutoStartDialog.h
    Sessions/ManageSessionsDialog.h
    Sessions/OpenSessionDialog.h
    HostAddressUtils.h
    Keyboard.h
    KeyboardThemeDialog.h
    MainWindow.h
    Preferences/PreferencesDialog.h
    ProcessDataStream.h
    Q3270.h
    Sessions/SaveSessionDialog.h
    Models/Session.h
    Models/KeyboardMap.h
    Sessions/SessionDialogBase.h
    Sessions/SessionPreviewWidget.h
    Stores/ColourStore.h
    Stores/KeyboardStore.h
    Stores/SessionStore.h
    SocketConnection.h
    Terminal.h
    Version.h
)

set(FORMS
    About.ui
    AutoStart.ui
    AutoStartAdd.ui
    CertificateDetails.ui
    ColourTheme.ui
    KeyboardTheme.ui
    MainWindowDialog.ui
    Preferences/ColourSwatchWidget.ui
    Preferences/KeyboardMapWidget.ui
    Sessions/ManageAutoStartDialog.ui
    Sessions/ManageSessions.ui
    NewTheme.ui
    Preferences/PreferencesDialog.ui
    Sessions/SessionDialog.ui
    Sessions/SessionPreview.ui
)

set(RESOURCES
    Q3270.qrc
)

if(Qt6_FOUND)
    qt_add_executable(Q3270
        ${SOURCES}
        ${HEADERS}
        ${FORMS}
        ${RESOURCES}
    )
    target_link_libraries(Q3270 PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Svg Qt6::SvgWidgets)
else()
    qt5_wrap_ui(UI_HEADERS ${FORMS})
    qt5_add_resources(RCC_SOURCES ${RESOURCES})
    add_executable(Q3270
        ${SOURCES}
        ${HEADERS}
        ${UI_HEADERS}
        ${RCC_SOURCES}
    )
    target_link_libraries(Q3270 PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Svg)
    # Ensure the build dir with ui_*.h is visible
    target_include_directories(Q3270 PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

target_include_directories(Q3270 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS Q3270 RUNTIME DESTINATION bin)
install(FILES LICENSE README.md UserGuide.md DESTINATION share/q3270)
