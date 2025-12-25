cmake_minimum_required(VERSION 3.20)

project(
    FRQS_WIDGET
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "High-Performance Modern C++ GUI Framework"
)

# ============================================================================
# 1. SETUP & OPTIONS
# ============================================================================
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Compiler Flags (Windows/MSVC Focus)
if(MSVC)
    add_compile_options(
        /W4 /permissive- 
        /Zc:__cplusplus 
        /std:c++latest 
        /EHsc /utf-8 /MP
    )
    add_compile_definitions(UNICODE _UNICODE NOMINMAX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -std=c++23)
endif()

# ============================================================================
# 2. SOURCE MANAGEMENT
# ============================================================================
# Note: CONFIGURE_DEPENDS makes GLOB safer by re-running CMake if files change
file(GLOB_RECURSE HEADERS_PUBLIC CONFIGURE_DEPENDS "include/*.hpp")
file(GLOB_RECURSE HEADERS_PRIVATE CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE SOURCES_IMPL CONFIGURE_DEPENDS "src/*.cpp")

# Exclude main.cpp from the Library (it's for the standalone executable only)
list(FILTER SOURCES_IMPL EXCLUDE REGEX ".*src/main.cpp$")

# ============================================================================
# 3. LIBRARY TARGET (THE CORE PRODUCT)
# ============================================================================
add_library(FRQS_WIDGET_LIB STATIC 
    ${HEADERS_PUBLIC} 
    ${HEADERS_PRIVATE} 
    ${SOURCES_IMPL}
)

# Alias for internal usage
add_library(FRQS::WIDGET_LIB ALIAS FRQS_WIDGET_LIB)

# Include Directories
target_include_directories(FRQS_WIDGET_LIB 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/src
)

# Dependencies (Direct2D & Windows)
if(WIN32)
    target_link_libraries(FRQS_WIDGET_LIB PUBLIC 
        d2d1 dwrite dwmapi shell32 windowscodecs
    )
endif()

# ============================================================================
# 4. EXECUTABLE TARGET (Internal App/Demo)
# ============================================================================
add_executable(FRQS_WIDGET src/main.cpp)

target_link_libraries(FRQS_WIDGET PRIVATE FRQS_WIDGET_LIB)

# ============================================================================
# 5. TESTING & EXAMPLES
# ============================================================================
option(BUILD_TESTS "Build test executables" ON)
option(BUILD_EXAMPLES "Build example executables" ON)

# Helper Macro for Examples
macro(create_frqs_example name source_file)
    add_executable(${name} ${source_file})
    target_link_libraries(${name} PRIVATE FRQS::WIDGET_LIB)
    if(MSVC)
        source_group("Examples" FILES ${source_file})
    endif()
endmacro()

if(BUILD_TESTS)
    enable_testing()
    # Simple test registration
    macro(create_frqs_test name source_file)
        add_executable(${name} ${source_file})
        target_link_libraries(${name} PRIVATE FRQS::WIDGET_LIB)
        add_test(NAME ${name} COMMAND ${name})
    endmacro()

    create_frqs_test(unit_tests         tests/unit_test.cpp)
    create_frqs_test(window_test        tests/window_test.cpp)
    create_frqs_test(flex_layout_test   tests/flex_layout_test.cpp)
endif()

if(BUILD_EXAMPLES)
    create_frqs_example(checkbox_demo           examples/checkbox_demo.cpp)
    # ... (Tambahkan example lain di sini kalau mau)
endif()

# ============================================================================
# 6. INSTALLATION & PACKAGING (The Enterprise Part)
# ============================================================================

# Install the Library
install(TARGETS FRQS_WIDGET_LIB
    EXPORT FRQSTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install Headers
install(DIRECTORY include/ 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Generate Config Files (FRQSConfig.cmake)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/FRQSConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Export Targets
install(EXPORT FRQSTargets
    FILE FRQSTargets.cmake
    NAMESPACE FRQS::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FRQS-Widget
)

# Install Config Files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/FRQSConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FRQS-Widget
)