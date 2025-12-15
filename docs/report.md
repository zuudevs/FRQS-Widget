report 1 - request
15-12-2025
1. Masalah Layar Putih (Blank Screen)
Deskripsi: Jendela aplikasi muncul tetapi tidak menampilkan konten widget apa pun (hanya putih polos). Penyebab Teknis:

Missing Renderer Instance: Pada file src/core/window_impl.hpp dan src/core/window.cpp, objek Window belum memiliki instance RendererD2D. Akibatnya, tidak ada objek yang bertugas menggambar grafik ke layar.

Empty Render Loop: Fungsi Application::renderWindows di src/core/application.cpp masih kosong (hanya berisi komentar TODO). Ini menyebabkan pipeline rendering tidak pernah dipanggil oleh main loop.

Overpainting (Z-Order Issue): Pada src/main.cpp, di dalam kelas ColoredWidget::render, pemanggilan Widget::render(renderer) dilakukan setelah kode gambar custom (fillRect, drawRect). Karena Widget::render menggambar latar belakang (background), gambar custom tertimpa dan tidak terlihat.

2. Masalah Glitch Saat Resize & Maximize
Deskripsi: Konten jendela terdistorsi (stretch), berkedip, atau tidak ter-update saat jendela diubah ukurannya atau digeser. Penyebab Teknis:

Unsynchronized Renderer Buffer: Pada fungsi Window::setSize di src/core/window.cpp, ukuran buffer renderer (Direct2D Render Target) tidak diperbarui (renderer->resize() tidak dipanggil) saat ukuran jendela fisik berubah. Ini menyebabkan ketidakcocokan antara ukuran jendela dan kanvas gambar.

Blocking Modal Loop: Saat jendela digeser atau di-resize oleh pengguna, Windows menahan thread utama dalam loop internal OS, sehingga main loop aplikasi Application::run berhenti sementara.

Missing Immediate Redraw: Fungsi Window::forceRedraw di src/core/window.cpp hanya memanggil invalidate() (menandai kotor) tapi tidak memaksa eksekusi render segera. Karena main loop sedang berhenti (poin sebelumnya), pembaruan layar tidak terjadi sampai geseran mouse dilepas.

3. Tombol Close Berjalan Normal
Analisis: Tombol Close bekerja normal karena event WM_CLOSE dan WM_DESTROY ditangani langsung oleh prosedur window (WndProc) dan mengirim pesan ke sistem event tanpa bergantung pada siklus render grafis.

report 1 - fixed 
15-12-2025

report 2 - request
15-12-2025
hasil dari tests\window_test.cpp
D:\Project\Fast Realibility Query System\FRQS Widget\build>"D:/Project/Fast Realibility Query System/FRQS Widget/build/window_test.exe"
Running test: window_creation
✓ Test passed: window_creation

Running test: window_visibility
✓ Test passed: window_visibility

Running test: window_resize
Assertion failed: actualSize.h != newSize.h