# FRQS-Widget: Technical Design Roadmap (Phase 1 & 2)

**Author:** Zuu 
**Date:** 2025-12-17  
**Status:** Draft / Planned  

Dokumen ini merinci rancangan implementasi untuk fitur-fitur krusial berikutnya: **Advanced Layout System**, **ScrollView**, dan **Toggle Widgets**.

---

## 1. Advanced Layout System ("Flexbox-Lite")

### Masalah Saat Ini
`StackLayout` saat ini hanya menumpuk widget secara linear. Tidak ada cara untuk membuat widget "mengisi sisa ruang" (fill/stretch) atau membagi ruang berdasarkan rasio (misal: 30% vs 70%).

### Solusi
Mengimplementasikan konsep **Weight (Flex Grow)** dan **Alignment** pada container.

### Perubahan Arsitektur

#### A. Penambahan `LayoutProps` pada `Widget`
Kita perlu cara untuk menyimpan parameter layout di setiap widget anak tanpa mengotori kelas `IWidget` utama.

**File:** `include/widget/widget.hpp`
```cpp
struct LayoutProps {
    float weight = 0.0f;        // 0 = fixed size, > 0 = share remaining space
    int32_t minSize = 0;        // Minimum width/height constraint
    int32_t maxSize = 99999;    // Maximum width/height constraint
    // Alignment hint (Start, Center, End, Stretch)
    enum class Align { Stretch, Start, Center, End } align = Align::Stretch; 
};

class Widget : public IWidget {
    // ...
    LayoutProps layoutProps_;
public:
    void setLayoutWeight(float weight) { layoutProps_.weight = weight; }
    float getLayoutWeight() const { return layoutProps_.weight; }
    // ... getters/setters lainnya
};