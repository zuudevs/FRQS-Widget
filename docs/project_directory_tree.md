FRQS_WIDGET/
├── CMakeLists.txt						   # [EXISTS]
├── .gitignore							   # [EXISTS]
├── include/
│   ├── frqs_widget.hpp                    # [EXISTS]
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
│   │   ├── window.hpp                     # [EXISTS]
│   │   ├── window_id.hpp                  # [EXISTS]
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
│   │   ├── application.cpp				   # [EXISTS]
│   │   ├── window_impl.hpp                # [EXISTS]
│   │   ├── window.cpp					   # [EXISTS]
│   │   └── window_registry.cpp			   # [EXISTS]
│   ├── event/
│   │   ├── event_bus.cpp
│   │   └── event_dispatcher.cpp
│   ├── widget/
│   │   └── widget.cpp					   # [EXISTS]
│   ├── render/
│   │   ├── renderer_d2d.hpp			   # [EXISTS]
│   │   ├── renderer_d2d.cpp			   # [EXISTS]
│   │   └── dirty_rect.cpp
│   └── platform/
│       ├── win32_window.cpp			   # [EXISTS] see error msg (1)
│       └── message_queue.cpp
├── tests/
│   ├── unit_test.cpp                      # [EXISTS]
│   ├── window_test.cpp
│   └── event_test.cpp
└── examples/
    └── hello_window.cpp

error msg :
(1)
- 'pImpl_' is a private member of 'frqs::core::Window'