project(graceful-panel LANGUAGES C)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GOBJECT REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(GTK4_X11 REQUIRED IMPORTED_TARGET gtk4-x11)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)

set(GRACEFUL_PANEL_LOGO_SOURCE_PATH "${CMAKE_SOURCE_DIR}/data/2.png")
set(GRACEFUL_PANEL_LOGO_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/graceful/panel")
set(GRACEFUL_PANEL_LOGO_INSTALL_PATH "${CMAKE_INSTALL_FULL_DATADIR}/graceful/panel/menu-logo.png")

add_library(graceful-panel-core STATIC
    panel-app-entry.c
    panel-app-entry.h
    panel-app-index.c
    panel-app-index.h
    panel-app.c
    panel-app.h
    panel-clock-model.c
    panel-clock-model.h
    panel-clock-item.c
    panel-clock-item.h
    panel-launcher-box.c
    panel-launcher-box.h
    panel-layout.c
    panel-layout.h
    panel-menu-button.c
    panel-menu-button.h
    panel-network-item.c
    panel-network-item.h
    panel-power-item.c
    panel-power-item.h
    panel-status-area.c
    panel-status-area.h
    panel-start-menu.c
    panel-start-menu.h
    panel-task-area.c
    panel-task-area.h
    panel-volume-item.c
    panel-volume-item.h
    panel-window.c
    panel-window.h
    panel-window-list.c
    panel-window-list.h
    panel-window-model.c
    panel-window-model.h
    panel-x11-protocol.c
    panel-x11-protocol.h
)
target_include_directories(graceful-panel-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(graceful-panel-core PRIVATE
    GRACEFUL_PANEL_LOGO_SOURCE_PATH="${GRACEFUL_PANEL_LOGO_SOURCE_PATH}"
    GRACEFUL_PANEL_LOGO_INSTALL_PATH="${GRACEFUL_PANEL_LOGO_INSTALL_PATH}"
)
target_link_libraries(graceful-panel-core PUBLIC
    PkgConfig::GLIB
    PkgConfig::GOBJECT
    PkgConfig::GIO
    PkgConfig::GTK4
    PkgConfig::GTK4_X11
    PkgConfig::X11
)

add_executable(graceful-panel main.c)
target_link_libraries(graceful-panel graceful-panel-core)

add_executable(panel-clock-model-test
    tests/test-panel-clock-model.c
)
target_link_libraries(panel-clock-model-test graceful-panel-core)
add_test(NAME panel-clock-model-test COMMAND panel-clock-model-test)

add_executable(panel-x11-protocol-test
    tests/test-panel-x11-protocol.c
)
target_link_libraries(panel-x11-protocol-test graceful-panel-core)
add_test(NAME panel-x11-protocol-test COMMAND panel-x11-protocol-test)

add_executable(panel-window-model-test
    tests/test-panel-window-model.c
)
target_link_libraries(panel-window-model-test graceful-panel-core)
add_test(NAME panel-window-model-test COMMAND panel-window-model-test)

add_executable(panel-app-entry-test
    tests/test-panel-app-entry.c
)
target_link_libraries(panel-app-entry-test graceful-panel-core)
add_test(NAME panel-app-entry-test COMMAND panel-app-entry-test)

install(TARGETS graceful-panel RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${GRACEFUL_PANEL_LOGO_SOURCE_PATH}
    DESTINATION ${GRACEFUL_PANEL_LOGO_INSTALL_DIR}
    RENAME menu-logo.png
)
