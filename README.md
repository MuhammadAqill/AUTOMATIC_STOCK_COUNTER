````markdown
# Automatic Stock Counter

**Automatic Stock Counter** merupakan projek tugasan akhir (FYP) yang automasi kaedah pengiraan stok menggunakan elektronik (microcontroller ESP-IDF) serta antaramuka pengguna yang mudah digunakan.

## Ciri Utama
- Mengesan dan mengira stok secara automatik.
- Dibangunkan menggunakan **ESP-IDF**, **C**, dan **CMake**.
- Struktur modular (komponen disusun mengikut direktori `/components` dan `/main`).
- Sesuai untuk integrasi sistem IoT atau automasi inventori.

## Struktur Projek
```text
├── components/           # Modul dan pustaka perincian
├── main/                 # Kod utama projek
├── CMakeLists.txt        # Konfigurasi binaan projek
├── sdkconfig             # Tetapan SDK ESP-IDF
└── README.md             # Fail dokumentasi ini
````

## Preparation & How to Build

1. Pasang **ESP-IDF** dan persediaan persekitaran pembangunan seperti yang disyorkan.
2. Klon repositori ke mesin anda:

   ```bash
   git clone https://github.com/MuhammadAqill/AUTOMATIC_STOCK_COUNTER.git
   ```
3. Masuk ke direktori projek:

   ```bash
   cd AUTOMATIC_STOCK_COUNTER
   ```
4. Jalankan perintah berikut untuk bina dan muat naik ke peranti ESP:

   ```bash
   idf.py build
   idf.py -p (PORT_SERIAL) flash monitor
   ```

## Cara Penggunaan

1. Pasang projek ke papan ESP menyambungkan ke:

   * Parent board
   * Sensor stok (contoh: sensor pembilang objek)
2. Jalankan firmware; papan akan mula mengira stok secara automatik.
3. Monitor output serial bagi:

   * Status semasa pengiraan stok
   * Notifikasi jika stok mencapai had tertentu

## Penambahbaikan Cadangan (Roadmap)

* Tambahkan sokongan **Wi-Fi/MQTT** untuk menghantar data stok ke pelayan jauh.
* Cipta **antaramuka pengguna web** atau aplikasi mudah alih untuk akses real-time.
* Pelbagaikan jenis sensor disokong (contoh: RFID, IR, ultrasonik).
* Log data pengiraan stok ke **SD Card** atau storan awan.
* Integrasi dengan sistem ERP untuk automasi pesanan.

## Penulis

**Muhammad Aqil**
Projek FYP ini dikembangkan pada tahun 2025.
