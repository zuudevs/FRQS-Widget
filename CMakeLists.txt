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
    include/core/window_id.hpp
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
    include/widget/button.hpp
    include/widget/container.hpp
    include/widget/label.hpp
    include/widget/slider.hpp
    include/widget/text_input.hpp
	include/widget/internal.hpp
	include/widget/scroll_view.hpp
    
    # Render
    include/render/dirty_rect.hpp
    include/render/renderer.hpp
    include/render/render_context.hpp
)

# ============================================================================
# PRIVATE HEADERS (Implementation details)
# ============================================================================

set(HEADERS_PRIVATE
    src/core/window_impl.hpp
    src/render/renderer_d2d.hpp
)

# ============================================================================
# SOURCE FILES (Implemented)
# ============================================================================

set(SOURCES_IMPL
    # Core
    src/core/application.cpp
    src/core/window.cpp
    src/core/window_registry.cpp

	# Event
    src/event/event_bus.cpp
    src/event/event_dispatcher.cpp
    
    # Widget
    src/widget/widget.cpp
    src/widget/button.cpp
    src/widget/container.cpp
    src/widget/label.cpp
    src/widget/slider.cpp
    src/widget/text_input.cpp
	src/widget/scroll_view.cpp
    
    # Render
    src/render/renderer_d2d.cpp
	src/render/dirty_rect.cpp
    
    # Platform
    src/platform/win32_window.cpp
	src/platform/message_queue.cpp
)

# ============================================================================
# MAIN EXECUTABLE
# ============================================================================

add_executable(FRQS_WIDGET
    src/main.cpp
    ${HEADERS_PUBLIC}
    ${HEADERS_PRIVATE}
    ${SOURCES_IMPL}
)

target_include_directories(FRQS_WIDGET
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# ============================================================================
# WINDOWS LIBRARIES
# ============================================================================

if(WIN32)
    target_link_libraries(FRQS_WIDGET
        PRIVATE
            d2d1        # Direct2D
            dwrite      # DirectWrite
            dwmapi      # Desktop Window Manager API (for invisible borders)
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
# STATIC LIBRARY (Optional)
# ============================================================================

add_library(FRQS_WIDGET_LIB STATIC
    ${HEADERS_PUBLIC}
    ${HEADERS_PRIVATE}
    ${SOURCES_IMPL}
)

target_include_directories(FRQS_WIDGET_LIB
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

if(WIN32)
    target_link_libraries(FRQS_WIDGET_LIB
        PUBLIC
            d2d1
            dwrite
            dwmapi
    )
    
    target_compile_definitions(FRQS_WIDGET_LIB
        PUBLIC
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
    # Unit test for basic types (Point, Size, Rect)
    add_executable(unit_tests
        tests/unit_test.cpp
    )
    
    target_link_libraries(unit_tests
        PRIVATE
            FRQS_WIDGET_LIB
    )
    
    target_include_directories(unit_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )
    
    # Window system test
    add_executable(window_test
        tests/window_test.cpp
    )
    
    target_link_libraries(window_test
        PRIVATE
            FRQS_WIDGET_LIB
    )
    
    target_include_directories(window_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )
    
    # Event system test
    add_executable(event_test
        tests/event_test.cpp
    )
    
    target_link_libraries(event_test
        PRIVATE
            FRQS_WIDGET_LIB
    )
    
    target_include_directories(event_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

	# Event system test
    add_executable(flex_layout_test
        tests/flex_layout_test.cpp
    )
    
    target_link_libraries(flex_layout_test
        PRIVATE
            FRQS_WIDGET_LIB
    )
    
    target_include_directories(flex_layout_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )
endif()

# ============================================================================
# EXAMPLES
# ============================================================================

option(BUILD_EXAMPLES "Build example executables" ON)

if(BUILD_EXAMPLES)
    # Hello Window Example
    add_executable(hello_window
        examples/hello_window.cpp
    )
    
    target_link_libraries(hello_window
        PRIVATE
            FRQS_WIDGET_LIB
    )
    
    target_include_directories(hello_window
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

	add_executable(widget_demo
        examples/widget_demo.cpp
    )
    
    target_link_libraries(widget_demo
        PRIVATE
            FRQS_WIDGET_LIB
    )
    
    target_include_directories(widget_demo
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

	add_executable(flex_layout
        examples/flex_layout_demo.cpp
    )
    
    target_link_libraries(flex_layout
        PRIVATE
            FRQS_WIDGET_LIB
    )
    
    target_include_directories(flex_layout
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

	# Scroll Demo
	add_executable(scroll_view_demo
		examples/scroll_view_demo.cpp
	)

	target_link_libraries(scroll_view_demo
		PRIVATE
			FRQS_WIDGET_LIB
	)

	target_include_directories(scroll_view_demo
		PRIVATE
			${CMAKE_SOURCE_DIR}/include
	)
endif()

# ============================================================================
# IDE SUPPORT (Visual Studio filters)
# ============================================================================

if(MSVC)
    # Public headers
    source_group(TREE ${CMAKE_SOURCE_DIR}/include 
                 PREFIX "Header Files/Public" 
                 FILES ${HEADERS_PUBLIC})
    
    # Private headers
    source_group(TREE ${CMAKE_SOURCE_DIR}/src 
                 PREFIX "Header Files/Private" 
                 FILES ${HEADERS_PRIVATE})
    
    # Source files
    source_group(TREE ${CMAKE_SOURCE_DIR}/src 
                 PREFIX "Source Files" 
                 FILES ${SOURCES_IMPL})
endif()

# ============================================================================
# INSTALLATION (Optional)
# ============================================================================

install(TARGETS FRQS_WIDGET FRQS_WIDGET_LIB
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# ============================================================================
# BUILD SUMMARY
# ============================================================================

message(STATUS "========================================")
message(STATUS "FRQS-Widget Configuration Summary")
message(STATUS "========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "========================================")
message(STATUS "Implementation Status:")
message(STATUS "  ✓ Headers: Complete")
message(STATUS "  ✓ Widget: Implemented")
message(STATUS "  ✓ Window: Implemented")
message(STATUS "  ✓ Application: Implemented")
message(STATUS "  ✓ Renderer: Implemented (Direct2D)")
message(STATUS "  ✓ Platform: Implemented (Win32)")
message(STATUS "========================================")
message(STATUS "Executables:")
message(STATUS "  - FRQS_WIDGET (main demo)")
if(BUILD_TESTS)
message(STATUS "  - unit_tests")
message(STATUS "  - window_test")
message(STATUS "  - event_test")
message(STATUS "  - flex_layout_test")
endif()
if(BUILD_EXAMPLES)
message(STATUS "  - hello_window")
message(STATUS "  - widget_demo")
message(STATUS "  - flex_layout")
message(STATUS "  - scroll_demo")
endif()
message(STATUS "========================================")