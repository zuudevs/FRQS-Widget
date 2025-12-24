cmake_minimum_required(VERSION 3.20)

project(
    FRQS_WIDGET
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "High-Performance Modern C++ GUI Framework"
)

# ============================================================================
# OUTPUT & DIRECTORIES
# ============================================================================
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin)
set(LIB_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# ============================================================================
# C++ STANDARD & FLAGS
# ============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /std:c++latest /EHsc /utf-8 /MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -std=c++23)
endif()

# ============================================================================
# FILE SOURCES
# ============================================================================
# Globbing is generally discouraged, but for conciseness in this specific context:
file(GLOB_RECURSE HEADERS_PUBLIC "include/*.hpp")
file(GLOB_RECURSE HEADERS_PRIVATE "src/*.hpp")
file(GLOB_RECURSE SOURCES_IMPL "src/*.cpp")

# Remove main.cpp from library sources to avoid entry point conflicts
list(FILTER SOURCES_IMPL EXCLUDE REGEX ".*src/main.cpp$")

# ============================================================================
# MAIN TARGETS
# ============================================================================

# 1. Main Executable
add_executable(FRQS_WIDGET src/main.cpp ${HEADERS_PUBLIC} ${HEADERS_PRIVATE} ${SOURCES_IMPL})

# 2. Static Library
add_library(FRQS_WIDGET_LIB STATIC ${HEADERS_PUBLIC} ${HEADERS_PRIVATE} ${SOURCES_IMPL})

# Common Configuration Macro
macro(configure_frqs_target target_name visibility)
    target_include_directories(${target_name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )

    if(WIN32)
        target_link_libraries(${target_name} ${visibility}
            d2d1 dwrite dwmapi shell32 windowscodecs
        )
        target_compile_definitions(${target_name} ${visibility}
            WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE
        )
    endif()
endmacro()

# Apply Config
configure_frqs_target(FRQS_WIDGET PRIVATE)
configure_frqs_target(FRQS_WIDGET_LIB PUBLIC)

# ============================================================================
# MACROS FOR TESTS & EXAMPLES (The Shortening Magic)
# ============================================================================

macro(create_frqs_test name source_file)
    add_executable(${name} ${source_file})
    target_link_libraries(${name} PRIVATE FRQS_WIDGET_LIB)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    # Add to IDE filter
    if(MSVC)
        source_group("Tests" FILES ${source_file})
    endif()
endmacro()

macro(create_frqs_example name source_file)
    add_executable(${name} ${source_file})
    target_link_libraries(${name} PRIVATE FRQS_WIDGET_LIB)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    # Add to IDE filter
    if(MSVC)
        source_group("Examples" FILES ${source_file})
    endif()
endmacro()

# ============================================================================
# BUILD TESTS
# ============================================================================
option(BUILD_TESTS "Build test executables" ON)

if(BUILD_TESTS)
    create_frqs_test(unit_tests         tests/unit_test.cpp)
    create_frqs_test(window_test        tests/window_test.cpp)
    create_frqs_test(event_test         tests/event_test.cpp)
    create_frqs_test(flex_layout_test   tests/flex_layout_test.cpp)
endif()

# ============================================================================
# BUILD EXAMPLES
# ============================================================================
option(BUILD_EXAMPLES "Build example executables" ON)

if(BUILD_EXAMPLES)
    create_frqs_example(window_demo             examples/window_demo.cpp)
    create_frqs_example(filedrop_demo           examples/filedrop_demo.cpp)
    create_frqs_example(widget_demo             examples/widget_demo.cpp)
    create_frqs_example(flex_layout             examples/flex_layout_demo.cpp)
    create_frqs_example(scrollview_demo         examples/scrollview_demo.cpp)
    create_frqs_example(image_and_checkbox_demo examples/image_and_checkbox_demo.cpp)
    create_frqs_example(checkbox_demo           examples/checkbox_demo.cpp)
    create_frqs_example(virtual_list_demo       examples/virtual_list_demo.cpp)
    create_frqs_example(combobox_demo           examples/combobox_demo.cpp)
    create_frqs_example(kitchen_sink_demo       examples/kitchen_sink_demo.cpp)
	create_frqs_example(button_demo       		examples/button_demo.cpp)
	create_frqs_example(layout_demo       		examples/layout_demo.cpp)
	create_frqs_example(dashboard_demo       	examples/dashboard_demo.cpp)
	create_frqs_example(setting_demo       		examples/setting_demo.cpp)
endif()

# IDE SUPPORT & INSTALL
# ============================================================================
if(MSVC)
    source_group(TREE ${CMAKE_SOURCE_DIR}/include PREFIX "Header Files/Public" FILES ${HEADERS_PUBLIC})
    source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Header Files/Private" FILES ${HEADERS_PRIVATE})
    source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SOURCES_IMPL})
endif()

install(TARGETS FRQS_WIDGET FRQS_WIDGET_LIB
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.hpp")

# ============================================================================
# SUMMARY
# ============================================================================
message(STATUS "------------------------------------------------")
message(STATUS "  FRQS-WIDGET CONFIGURATION (v${PROJECT_VERSION})")
message(STATUS "------------------------------------------------")
message(STATUS "  C++ Standard : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Output Dir   : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Tests        : ${BUILD_TESTS}")
message(STATUS "  Examples     : ${BUILD_EXAMPLES}")
message(STATUS "------------------------------------------------")