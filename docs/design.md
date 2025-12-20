# FRQS-Widget: Filosofi Desain & Standar Arsitektur

> **Motto:** *"Cepat, Ringan, Aman, Efektif."*

---

## 1. Tujuan Proyek (The Mission)

Membangun sebuah pustaka **GUI (Graphical User Interface)** berbasis **C++23** yang:

* **High-Performance**  
  Mampu menangani ribuan widget dan update UI **60 FPS** tanpa membebani CPU/GPU.

* **Developer-Centric**  
  Mudah digunakan (API bersih), namun tetap memberikan akses ke fitur *low-level* jika diperlukan.

* **Modern Standard**  
  Menjadi contoh implementasi fitur C++ modern (*Modules, Concepts, Smart Pointers*) dalam pengembangan aplikasi desktop Windows.

* **Zero-Bloat**  
  Hanya menyertakan fitur esensial tanpa dependensi pihak ketiga yang berat (selain **Windows SDK**).

---

## 2. Prinsip Inti (Core Principles)

Setiap baris kode yang ditulis **wajib** mematuhi prinsip berikut.

### A. The "Hot Process" Rule (Hukum Proses Panas)

**Definisi:**  
Kode yang dieksekusi berulang kali dalam satu frame (*render loop, kalkulasi geometri*).

**Aturan:**

* **NO Virtuals**  
  Dilarang menggunakan *virtual function* pada objek kecil yang sering diakses (`Point`, `Rect`, `Color`).  
  Gunakan **Template** dan **Concepts**.

* **Zero Allocation**  
  Dilarang melakukan alokasi heap (`new`, `malloc`) di dalam `render()` atau `onEvent()`.  
  Gunakan *stack allocation* atau *object pooling*.

---

### B. Keamanan & Enkapsulasi (Safety First)

* **Hidden HWND**  
  Handle native (`HWND`) adalah *racun* bagi portabilitas dan keamanan.  
  Sembunyikan di balik **PImpl** (`Window::Impl`).

* **Explicit Backdoor**  
  Jika user membutuhkan akses *low-level*, paksa melalui namespace eksplisit:  \

  ```cpp
  frqs::core::unsafe
  ```

  Tujuannya agar developer sadar terhadap risiko yang diambil.

* **Macro Hygiene**  
  Header **tidak boleh** mencemari global namespace.  
  Selalu `#undef` makro Windows yang umum konflik:

  * `CreateWindow`
  * `DrawText`
  * `min`, `max`

---

### C. Efisiensi Sumber Daya (Resource Efficiency)

* **Dirty Rects**  
  Jangan gambar ulang seluruh layar jika hanya satu widget berubah.  
  Gunakan sistem **DirtyRectManager** untuk *partial redraw*.

* **Event-Driven**  
  Aplikasi harus *tidur* saat tidak ada input.  
  Gunakan `WaitMessage()` atau *timer* efisien — **bukan busy loop**.

* **Hybrid Event System**

  | Jenis Event              | Mekanisme      | Alasan                            |
  | ------------------------ | -------------- | --------------------------------- |
  | Event kecil (Mouse, Key) | Value (Stack)  | Cache-friendly                    |
  | Event besar (FileDrop)   | Pointer (Heap) | Hindari `std::variant` membengkak |

  **Tujuan:** menjaga ukuran `std::variant` tetap kecil dan efisien.

---

### D. Modularitas & Skalabilitas (Scalable Architecture)

* **PImpl Idiom**  
  Pisahkan deklarasi (`.hpp`) dan implementasi (`.cpp`) secara ketat untuk mempercepat waktu kompilasi.

* **Interface Segregation**  
  Gunakan interface (`IListAdapter`, `IWidget`) untuk memisahkan logika data dan tampilan.

* **Single Responsibility**  
  Satu kelas, satu tugas:

  * `Window` → OS & lifecycle
  * `Renderer` → Drawing & raster
  * `Widget` → Logika UI

---

## 3. Standar Teknis (Technical Standards)

### Bahasa & Compiler

* **Standard:** C++23 *(Wajib)*
* **Compiler:** MSVC 19.38+ (Visual Studio 2022 v17.8+)
* **Compiler Flags:**

  * `/std:c++latest`
  * `/permissive-`
  * Warning Level **4**

---

### Naming Convention

* **Namespace:** `frqs::widget` *(alias: `frqs::ui`)*
* **Classes / Structs:** `PascalCase`  
  Contoh: `EventBus`, `TextInput`
* **Functions / Methods:** `camelCase`  
  Contoh: `setRect()`, `onEvent()`
* **Variables:** `camelCase`  
  Contoh: `buttonCount`
* **Private Members:** `camelCase_` *(underscore suffix)*  
  Contoh: `pImpl_`, `isVisible_`
* **Macros:** `UPPER_SNAKE_CASE`  
  Contoh: `FRQS_POST_UI`

---

### Struktur Direktori

```text
include/    # Header publik (API)
src/        # Implementasi internal
examples/   # Demo penggunaan
tests/      # Unit testing
docs/       # Dokumentasi teknis
```

---

## Penutup

Dokumen ini merupakan **kontrak teknis** antara:

* **Pengembang:** Rara  \
* **Arsitek Sistem:** Zuu

Seluruh kontribusi ke **FRQS-Widget** wajib mengikuti filosofi dan standar yang tertulis di dokumen ini.
