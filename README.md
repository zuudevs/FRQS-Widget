# FRQS-Widget

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)
![Platform: Windows](https://img.shields.io/badge/Platform-Windows%2010%2F11-blue.svg)
![Rendering: Direct2D](https://img.shields.io/badge/Rendering-Direct2D%201.3-orange.svg)
![Build: CMake](https://img.shields.io/badge/Build-CMake%203.20%2B-brightgreen.svg)
![Status: Alpha](https://img.shields.io/badge/Status-Alpha-red.svg)

**Framework GUI C++23 Kinerja Tinggi**

* **Versi**: 0.1.0 (Alpha)
* **Lisensi**: MIT
* **Platform**: Windows 10 / 11
* **Backend Rendering**: Direct2D 1.3

---

## 1. Tinjauan Proyek

**FRQS-Widget** adalah pustaka GUI (*Graphical User Interface*) modern yang dibangun di atas standar **C++23**. Framework ini dirancang dengan fokus pada **kinerja tinggi**, **keamanan memori**, dan **kemudahan penggunaan**, tanpa mengorbankan akses ke fitur *low-level* bila diperlukan.

Framework ini menyembunyikan kompleksitas Win32 API (seperti `HWND` dan *message loop*) dari pengguna akhir, namun tetap menyediakan jalur eksplisit bagi pengembang tingkat lanjut.

---

## 2. Tujuan Utama (Goals)

* **High Performance**
  Meminimalkan alokasi heap pada *render loop* dan menghindari overhead *virtual function* pada *hot path* (misalnya perhitungan geometri).

* **Modern & Safe**
  Menggunakan fitur modern C++ seperti **Concepts**, **Smart Pointers**, dan **Variants** untuk menjamin *type safety* dan *memory safety*.

* **Lightweight**
  Tidak menggunakan dependensi pihak ketiga yang berat. Hanya bergantung pada **Windows SDK standar**.

* **Developer Friendly**
  API bersih, modular, dan mudah digunakan, tanpa memaparkan detail Win32 API secara langsung.

---

## 3. Prinsip Inti & Standar Kode

### A. Aturan "Hot Process"

* **Virtual Functions**
  Digunakan hanya pada level arsitektur tinggi (misalnya `IWidget`, `IControl`).

* **No Virtuals in Hot Path**
  Objek kecil yang sering diakses (seperti `Point`, `Rect`, `Color`) menggunakan *Templates* dan *Concepts*, bukan polimorfisme runtime.

* **Memory Allocation**
  Hindari `new` / `malloc` di dalam *render loop*. Gunakan *stack allocation* atau *object pooling*.

### B. Keamanan & Enkapsulasi

* **Hidden HWND**
  Handle native (`HWND`) bersifat privat dan tidak dapat diakses secara tidak sengaja oleh pengguna.

* **Low-level Backdoor**
  Akses tingkat rendah hanya tersedia melalui namespace eksplisit:

  ```cpp
  frqs::core::unsafe
  ```

* **Macro Safety**
  Header library secara otomatis menangani polusi makro Windows:

  * `#undef CreateWindow`
  * `#define NOMINMAX`

### C. Konvensi Penamaan

* **Namespace**: `frqs::widget` (alias: `frqs::ui`)
* **Tipe / Kelas**: `PascalCase` (contoh: `TextInput`, `EventBus`)
* **Fungsi / Variabel**: `camelCase` (contoh: `setRect`, `onEvent`)
* **Anggota Privat**: diakhiri dengan underscore (`_`) (contoh: `pImpl_`, `size_`)

---

## 4. Technology Stack

| Komponen     | Teknologi    | Deskripsi                                                         |
| ------------ | ------------ | ----------------------------------------------------------------- |
| Bahasa       | C++23        | Menggunakan `std::expected`, *concepts*, dan fitur modern lainnya |
| Build System | CMake 3.20+  | Konfigurasi build modular                                         |
| Rendering    | Direct2D 1.3 | Rendering 2D terakselerasi hardware                               |
| Mesin Teks   | DirectWrite  | Rendering & layout teks berkualitas tinggi                        |
| OS Backend   | Win32 API    | Windowing & input native (disembunyikan)                          |

---

## 5. Sorotan Arsitektur

### A. Hybrid Event System

Sistem event menggunakan `std::variant` untuk *type safety* dengan optimasi memori:

* **Hot Events** (Mouse, Keyboard)
  Disimpan sebagai *value type* (struct) di dalam `std::variant` (*stack-based*, tanpa alokasi heap).

* **Cold Events** (FileDrop, Paste)
  Disimpan sebagai `std::unique_ptr` (*heap-based*) untuk menjaga ukuran `std::variant` tetap kecil (≤ 64 byte).

### B. Pipeline Rendering (Dirty Rects)

1. Widget menandai dirinya sebagai kotor menggunakan `invalidate()`
2. `DirtyRectManager` mengumpulkan area yang berubah
3. Renderer hanya menggambar ulang area tersebut
4. Direct2D melakukan *clipping* pada area yang relevan

### C. Sistem Layout (Flexbox Lite)

* **StackLayout**
  Penumpukan widget secara vertikal atau horizontal.

* **FlexLayout** *(dalam pengembangan)*
  Sistem layout berbasis *weight* dan *alignment*, terinspirasi dari CSS Flexbox, menggunakan struktur ringan `LayoutProps`.

### D. Idiom PImpl (*Pointer to Implementation*)

Digunakan pada hampir semua kelas utama (`Widget`, `Window`, `Application`).

**Manfaat:**

* Mengurangi waktu kompilasi
* Menyembunyikan `windows.h` dari *public API*

---

## 6. Struktur Proyek

```
FRQS_WIDGET/
├── include/                  # Public Headers (API)
│   ├── core/                 # Window & Application logic
│   ├── event/                # Event system & Event Bus
│   ├── widget/               # Widget (Button, Label, dll.)
│   ├── render/               # Renderer interfaces
│   ├── unit/                 # Geometry (Point, Rect, Color)
│   └── platform/             # Platform abstractions
├── src/                      # Implementation
│   ├── platform/             # Win32-specific code
│   ├── render/               # Direct2D implementation
│   └── ...
├── examples/                 # Contoh penggunaan
└── tests/                    # Unit tests
```

---

## 7. Memulai

### Prasyarat

* Visual Studio 2022 (v17.8+ untuk dukungan C++23)
* CMake 3.20+

### Membangun (Build)

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## 8. Contoh Penggunaan Dasar

```cpp
#include <frqs-widget.hpp>

int main() {
    using namespace frqs;

    // 1. Initialize application
    auto& app = Application::instance();
    app.initialize();

    // 2. Create window
    WindowParams params;
    params.title = L"Aplikasi Pertama Saya";
    params.size = Size(800u, 600u);
    auto window = app.createWindow(params);

    // 3. Setup UI
    auto container = widget::createVStack();
    auto button = std::make_shared<widget::Button>(L"Klik Saya!");
    container->addChild(button);

    window->setRootWidget(container);
    window->show();

    // 4. Run loop
    app.run();
    return 0;
}
```

---

## 9. Informasi Dokumentasi

Dokumentasi dibuat oleh **FRQS Team**
Terakhir diperbarui: **17-12-2025**
