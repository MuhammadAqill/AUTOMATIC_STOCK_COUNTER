#!/usr/bin/env python3
"""
stock_counter_plot_gram_integer.py

Program untuk plot bacaan berat (Gram).
- Graph hanya keluar selepas semua input selesai
- Garis sambungan berwarna putih
- Semua titik input user berwarna merah
- Perubahan kecil pada perpuluhan diabaikan
- Graph hanya ikut nilai integer (floor)
"""

import plotext as plt

SAMPLES = 20


def simple_plot(data, title="Data (Gram Integer)"):
    plt.clf()
    x = list(range(1, len(data) + 1))

    # Ambil hanya integer (abaikan perpuluhan)
    int_data = [int(v) for v in data]

    # Plot garis putih
    plt.plot(x, int_data, marker="none", color="white")

    # Plot semua titik merah
    plt.scatter(x, int_data, marker="dot", color="red")

    plt.title(title)
    plt.xlabel("Sample index")
    plt.ylabel("Berat (gram, integer)")

    # Skala sederhana
    if int_data:
        y_min = min(int_data)
        y_max = max(int_data)
        margin = (y_max - y_min) * 0.2
        if margin == 0:
            margin = 1
        y_min -= margin
        y_max += margin
        plt.ylim(y_min, y_max)

    plt.show()


def manual_gram_samples():
    vals = []
    print(f"Masukkan {SAMPLES} nilai Gram (tekan Enter selepas setiap nilai).")
    for i in range(SAMPLES):
        while True:
            s = input(f"Gram sample {i+1}: ").strip()
            try:
                v = float(s)
                vals.append(v)
                break
            except ValueError:
                print("⚠️ Sila masukkan nombor sah.")
    return vals


def main():
    print("=== Graph Bacaan Gram (integer sahaja) ===")

    grams = manual_gram_samples()

    avg_gram = sum(grams) / len(grams)
    print("\n=== RESULT ===")
    print("Purata Gram (asli):", round(avg_gram, 2))
    print("Purata Gram (integer):", round(sum(int(v) for v in grams) / len(grams), 0))

    # Paparkan graph selepas semua data dimasukkan
    print("\nGraph of measured Gram values (integer):")
    simple_plot(grams, "Measured Gram (Integer)")


if __name__ == "__main__":
    main()
