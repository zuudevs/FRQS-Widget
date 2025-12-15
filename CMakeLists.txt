cmake_minimum_required(VERSION 3.20)

project(
    FRQS_WIDGET
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "High-Performance Modern C++ GUI Framework"
)

# ============================================================================
# C++ STANDARD
# ============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# COMPILER FLAGS
# ============================================================================
if(MSVC)
    add_compile_options(
        /W4                 # Warning level 4
        /permissive-        # Standards conformance
        /Zc:__cplusplus     # Correct __cplusplus macro
        /std:c++latest      # Latest C++ standard
        /EHsc               # Exception handling
        /utf-8              # UTF-8 source and execution
    )
    
    # Optional: Enable faster builds
    add_compile_options(/MP)  # Multi-processor compilation
    
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -std=c++23
    )
endif()

# ============================================================================
# HEADER FILES
# ============================================================================

set(HEADERS_PUBLIC
    # Main header
    include/frqs-widget.hpp
    
    # Meta
    include/meta/concepts.hpp
    
    # Units (existing)
    include/unit/point.hpp
    include/unit/size.hpp
    include/unit/rect.hpp
    include/unit/color.hpp
    include/unit/ipair.hpp
    
    # Platform
    include/platform/win32_safe.hpp
    include/platform/message_queue.hpp
    
    # Core
    include/core/application.hpp
    include/core/window.hpp
    include/core/window_registry.hpp
    include/core/native_handle.hpp
    
    # Event
    include/event/event.hpp
    include/event/event_types.hpp
    include/event/event_bus.hpp
    
    # Widget
    include/widget/iwidget.hpp
    include/widget/widget.hpp
    include/widget/layout.hpp
    
    # Render
    include/render/dirty_rect.hpp
    include/render/renderer.hpp
    include/render/render_context.hpp
)

# ============================================================================
# SOURCE FILES (To be implemented)
# ============================================================================

set(SOURCES_IMPL
    # Platform (to be implemented)
    # src/platform/win32_window.cpp
    # src/platform/message_queue.cpp
    
    # Core (to be implemented)
    # src/core/application.cpp
    # src/core/window.cpp
    # src/core/window_registry.cpp
    
    # Event (to be implemented)
    # src/event/event_bus.cpp
    
    # Widget (to be implemented)
    # src/widget/widget.cpp
    
    # Render (to be implemented)
    # src/render/dirty_rect.cpp
    # src/render/renderer_d2d.cpp
)

# ============================================================================
# MAIN EXECUTABLE (Header-only for now)
# ============================================================================

add_executable(FRQS_WIDGET
    src/main.cpp
    ${HEADERS_PUBLIC}
)

target_include_directories(FRQS_WIDGET
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ============================================================================
# WINDOWS LIBRARIES
# ============================================================================

if(WIN32)
    target_link_libraries(FRQS_WIDGET
        PRIVATE
            d2d1        # Direct2D
            dwrite      # DirectWrite
    )
    
    # Define WIN32_LEAN_AND_MEAN and NOMINMAX
    target_compile_definitions(FRQS_WIDGET
        PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            UNICODE
            _UNICODE
    )
endif()

# ============================================================================
# TESTS (Optional)
# ============================================================================

option(BUILD_TESTS "Build test executables" ON)

if(BUILD_TESTS)
    add_executable(unit_tests
        tests/unit_test.cpp
        ${HEADERS_PUBLIC}
    )
    
    target_include_directories(unit_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )
    
    if(WIN32)
        target_link_libraries(unit_tests PRIVATE d2d1 dwrite)
        target_compile_definitions(unit_tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    endif()
endif()

# ============================================================================
# IDE SUPPORT (Visual Studio filters)
# ============================================================================

if(MSVC)
    source_group(TREE ${CMAKE_SOURCE_DIR}/include PREFIX "Header Files" FILES ${HEADERS_PUBLIC})
    source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SOURCES_IMPL})
endif()

# ============================================================================
# INSTALLATION (Optional)
# ============================================================================

install(TARGETS FRQS_WIDGET
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)