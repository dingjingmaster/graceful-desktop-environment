project(graceful-session LANGUAGES C)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GOBJECT REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)

add_library(graceful-session-core STATIC
    session-component.c
    session-component.h
    session-definition.c
    session-definition.h
    session-environment.c
    session-environment.h
    session-autostart.c
    session-autostart.h
    session-process.c
    session-process.h
    session-manager.c
    session-manager.h
)
target_include_directories(graceful-session-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(graceful-session-core PRIVATE
    GRACEFUL_SESSION_BINDIR="${CMAKE_INSTALL_FULL_BINDIR}"
)
target_link_libraries(graceful-session-core PUBLIC PkgConfig::GLIB PkgConfig::GOBJECT PkgConfig::GIO)

add_executable(graceful-session main.c)
target_link_libraries(graceful-session graceful-session-core)

add_executable(session-definition-test
    tests/test-session-definition.c
)
target_link_libraries(session-definition-test graceful-session-core)
add_test(NAME session-definition-test COMMAND session-definition-test)

add_executable(session-environment-test
    tests/test-session-environment.c
)
target_link_libraries(session-environment-test graceful-session-core)
add_test(NAME session-environment-test COMMAND session-environment-test)

add_executable(session-component-test
    tests/test-session-component.c
)
target_link_libraries(session-component-test graceful-session-core)
add_test(NAME session-component-test COMMAND session-component-test)

add_executable(session-autostart-test
    tests/test-session-autostart.c
)
target_link_libraries(session-autostart-test graceful-session-core)
add_test(NAME session-autostart-test COMMAND session-autostart-test)

add_executable(session-process-test
    tests/test-session-process.c
)
target_link_libraries(session-process-test graceful-session-core)
add_test(NAME session-process-test COMMAND session-process-test)

install(TARGETS graceful-session RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES graceful.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/wayland-sessions)
