FRQS_WIDGET/
├── CMakeLists.txt						   # [EXISTS]
├── .gitignore							   # [EXISTS]
├── include/
│   ├── frqs_widget.hpp                    # [EXISTS] but error see error msg (1)
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
│   │   ├── window.hpp                     # [EXISTS] but error see error msg (1)
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
│   │   ├── application.cpp				   # [EXISTS] but error see error msg (1)
│   │   ├── window_impl.hpp                # [EXISTS] but error see error msg (1)
│   │   ├── window.cpp					   # [EXISTS] but error see error msg (1)
│   │   └── window_registry.cpp			   # [EXISTS]
│   ├── event/
│   │   ├── event_bus.cpp
│   │   └── event_dispatcher.cpp
│   ├── widget/
│   │   └── widget.cpp
│   ├── render/
│   │   ├── renderer_d2d.hpp			   # [EXISTS]
│   │   ├── renderer_d2d.cpp			   # [EXISTS]
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

error msg :
(1) :
- In included file: friend declaration of 'getNativeHandle' does not match any declaration in namespace 'frqs::core::unsafe'clang(qualified_friend_no_match)
- window.hpp(16, 11): Candidate function has type mismatch at 1st parameter (expected 'const Window *' but has 'const class Window *')