FRQS_WIDGET/
├── CMakeLists.txt
├── .gitignore
├── include/
│   ├── frqs_widget.hpp                    # [EXISTS] but error core/window not implemented
│   ├── meta/
│   │   ├── arithmetic.hpp                 # [DELETED]
│   │   └── concepts.hpp                   # [EXISTS]
│   ├── unit/
│   │   ├── point.hpp                      # [EXISTS]
│   │   ├── size.hpp                       # [EXISTS]
│   │   ├── rect.hpp                       # [EXISTS]
│   │   ├── color.hpp                      # [EXISTS]
│   │   └── basic_paired.hpp               # [EXISTS]
│   ├── core/
│   │   ├── application.hpp                # [EXISTS]
│   │   ├── window.hpp                     # [EXISTS] but error not implemeted, only declared
│   │   ├── window_registry.hpp            # [EXISTS]
│   │   └── native_handle.hpp              # [EXISTS]
│   ├── event/
│   │   ├── event.hpp                      # [EXISTS]
│   │   ├── event_bus.hpp                  # [EXISTS]
│   │   └── event_types.hpp                # [EXISTS]
│   ├── widget/
│   │   ├── iwidget.hpp                    # [EXISTS]
│   │   ├── widget.hpp                     # [EXISTS]
│   │   └── layout.hpp                     # [EXISTS]
│   ├── render/
│   │   ├── renderer.hpp                   # [EXISTS]
│   │   ├── render_context.hpp             # [EXISTS]
│   │   └── dirty_rect.hpp                 # [EXISTS]
│   └── platform/
│       ├── win32_safe.hpp                 # [EXISTS]
│       └── message_queue.hpp              # [EXISTS]
├── src/
│   ├── core/
│   │   ├── application.cpp
│   │   ├── window_impl.hpp                # PImpl details
│   │   ├── window.cpp
│   │   └── window_registry.cpp
│   ├── event/
│   │   ├── event_bus.cpp
│   │   └── event_dispatcher.cpp
│   ├── widget/
│   │   └── widget.cpp
│   ├── render/
│   │   ├── renderer_d2d.hpp               # Direct2D implementation
│   │   ├── renderer_d2d.cpp
│   │   └── dirty_rect.cpp
│   └── platform/
│       ├── win32_window.cpp
│       └── message_queue.cpp
├── tests/
│   ├── unit_test.cpp                      # [EXISTS]
│   ├── window_test.cpp
│   └── event_test.cpp
└── examples/
    └── hello_window.cpp