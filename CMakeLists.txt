cmake_minimum_required(VERSION 3.20)
project(PKKM VERSION 0.1.2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_library(pkkm_core src/kingdom.cpp src/building_catalog.cpp src/greenbelt.cpp)
target_include_directories(pkkm_core PUBLIC include PRIVATE third_party)
if(MSVC)
 target_compile_options(pkkm_core PRIVATE /W4 /permissive-)
else()
 target_compile_options(pkkm_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_executable(pkkm src/main.cpp)
target_link_libraries(pkkm PRIVATE pkkm_core)
target_compile_definitions(pkkm PRIVATE PKKM_VERSION="${PROJECT_VERSION}")
add_custom_command(TARGET pkkm POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy_if_different
 "${CMAKE_CURRENT_SOURCE_DIR}/assets/building_catalog.json"
 "$<TARGET_FILE_DIR:pkkm>/building_catalog.json")
 add_custom_command(TARGET pkkm POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kingmaker-Greenbelt-Map.jpg"
  "$<TARGET_FILE_DIR:pkkm>/Kingmaker-Greenbelt-Map.jpg")
if(WIN32)
 add_executable(pkkm_desktop WIN32 src/windows_app.cpp src/map_editor.cpp)
 target_link_libraries(pkkm_desktop PRIVATE pkkm_core comctl32 gdiplus)
 add_custom_command(TARGET pkkm_desktop POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/building_catalog.json"
  "$<TARGET_FILE_DIR:pkkm_desktop>/building_catalog.json")
 add_custom_command(TARGET pkkm_desktop POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kingmaker-Greenbelt-Map.jpg"
  "$<TARGET_FILE_DIR:pkkm_desktop>/Kingmaker-Greenbelt-Map.jpg")
endif()
enable_testing()
add_executable(pkkm_tests tests/kingdom_tests.cpp)
target_link_libraries(pkkm_tests PRIVATE pkkm_core)
target_include_directories(pkkm_tests PRIVATE third_party)
add_test(NAME kingdom_domain COMMAND pkkm_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/building_catalog.json")
find_program(PKKM_PYTHON_EXECUTABLE NAMES python python3)
if(PKKM_PYTHON_EXECUTABLE)
    add_test(NAME python_suite
        COMMAND "${PKKM_PYTHON_EXECUTABLE}" -m unittest discover -s tests -p "test_*.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Install/CPack payload for a reproducible end-user distribution. Tests and
# source-only importer utilities intentionally remain outside the package.
include(GNUInstallDirs)
install(TARGETS pkkm
    RUNTIME DESTINATION .
)
if(WIN32)
    install(TARGETS pkkm_desktop
        RUNTIME DESTINATION .
    )
endif()
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/building_catalog.json"
    DESTINATION .
)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kingmaker-Greenbelt-Map.jpg"
    DESTINATION .
)

set(CPACK_PACKAGE_NAME "PKKM")
set(CPACK_PACKAGE_VENDOR "PKKM")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pathfinder Kingdom Manager")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "PKKM-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
