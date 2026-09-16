project(graceful-greeter LANGUAGES C)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(GTK4_X11 REQUIRED IMPORTED_TARGET gtk4-x11)
pkg_check_modules(GOBJECT REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(LIGHTDM_GOBJECT REQUIRED IMPORTED_TARGET liblightdm-gobject-1)

add_library(graceful-greeter-core STATIC
    greeter-config.c
    greeter-config.h
    greeter-login-model.c
    greeter-login-model.h
)
target_include_directories(graceful-greeter-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(graceful-greeter-core PUBLIC PkgConfig::GOBJECT)

add_executable(graceful-greeter main.c)
target_link_libraries(graceful-greeter
    graceful-common
    graceful-greeter-core
    PkgConfig::GTK4
    PkgConfig::GTK4_X11
    PkgConfig::LIGHTDM_GOBJECT
)

add_executable(greeter-login-model-test
    tests/test-login-model.c
)
target_link_libraries(greeter-login-model-test graceful-greeter-core)
add_test(NAME greeter-login-model-test COMMAND greeter-login-model-test)

add_executable(greeter-config-test
    tests/test-config.c
)
target_link_libraries(greeter-config-test graceful-greeter-core)
add_test(NAME greeter-config-test COMMAND greeter-config-test)

install(TARGETS graceful-greeter RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES graceful-greeter.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/xgreeters)
