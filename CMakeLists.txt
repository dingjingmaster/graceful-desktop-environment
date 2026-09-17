cmake_minimum_required(VERSION 3.20)
project(graceful-desktop-environment LANGUAGES C)

enable_testing()

add_subdirectory(common)
add_subdirectory(desktop)
add_subdirectory(greeter)
add_subdirectory(session)
