project(graceful-desktop LANGUAGES C)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GOBJECT REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_library(graceful-desktop-core STATIC
    desktop-app.c
    desktop-app.h
    desktop-config.c
    desktop-config.h
    wallpaper-transition.c
    wallpaper-transition.h
    wallpaper-store.c
    wallpaper-store.h
    wallpaper-view.c
    wallpaper-view.h
)
target_include_directories(graceful-desktop-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(graceful-desktop-core PUBLIC PkgConfig::GLIB PkgConfig::GOBJECT PkgConfig::GIO PkgConfig::GTK4)

add_executable(graceful-desktop main.c)
target_link_libraries(graceful-desktop graceful-desktop-core)

add_executable(desktop-config-test
    tests/test-desktop-config.c
)
target_link_libraries(desktop-config-test graceful-desktop-core)
add_test(NAME desktop-config-test COMMAND desktop-config-test)

add_executable(wallpaper-store-test
    tests/test-wallpaper-store.c
)
target_link_libraries(wallpaper-store-test graceful-desktop-core)
add_test(NAME wallpaper-store-test COMMAND wallpaper-store-test)

add_executable(wallpaper-transition-test
    tests/test-wallpaper-transition.c
)
target_link_libraries(wallpaper-transition-test graceful-desktop-core)
add_test(NAME wallpaper-transition-test COMMAND wallpaper-transition-test)

install(TARGETS graceful-desktop RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
