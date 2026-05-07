import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# --- PARAMÉTEREK (Az Arduino értékeihez igazítva) ---
#FILE_TO_CHECK = "zaj.csv" 
FILE_TO_CHECK = "zaj.csv"
ALPHA = 0.15          # filterWeight
K = 1.1               # SENSITIVITY
MIN_THRESHOLD = 0.5  # MIN_THRESHOLD
NOISE_ALPHA = 0.001   # Az Arduinóban fixen ennyi

DELIMITER = "|"

# Adatok beolvasása
# Oszlopok: timestamp, w (alpha), t (threshold), e (event), f (filtered), raw, total
df = pd.read_csv(FILE_TO_CHECK, sep=DELIMITER, names=['timestamp', 'w', 't', 'e', 'f', 'raw', 'total'], header=0)
raw_data = df['raw'].values

# Szimulációs tömbök a grafikonhoz
filtered = np.zeros_like(raw_data)
threshold_history = np.zeros_like(raw_data)
noise_history = np.zeros_like(raw_data)

events_idx = []
curr_f = 0
avg_noise = 0.5  # Kezdőérték, mint a kontrolleren
last_p = 0

# Szimuláció futtatása
for i in range(len(raw_data)):
    # 1. Jel szűrése (EMA)
    curr_f = (raw_data[i] * ALPHA) + (curr_f * (1.0 - ALPHA))
    filtered[i] = curr_f
    
    # 2. Zajszint (jitter) becslése
    jitter = abs(raw_data[i] - curr_f)
    avg_noise = (jitter * NOISE_ALPHA) + (avg_noise * (1.0 - NOISE_ALPHA))
    noise_history[i] = avg_noise
    
    # 3. Dinamikus küszöb kiszámítása
    dynamic_t = (avg_noise * K) + MIN_THRESHOLD
    if dynamic_t > 30: dynamic_t = 30
    threshold_history[i] = dynamic_t
    
    # 4. Esemény detektálása
    if abs(curr_f - last_p) >= dynamic_t:
        events_idx.append(i)
        last_p = curr_f

# --- EREDMÉNY KIÍRÁSA ---
print(f"Fájl: {FILE_TO_CHECK}")
print(f"Paraméterek: Alpha={ALPHA}, K={K}, Min_T={MIN_THRESHOLD}")
print(f"Detektált események száma: {len(events_idx)}")

# Vizuális ellenőrzés
plt.figure(figsize=(14, 7))

# Felső grafikon: Jel és események
ax1 = plt.subplot(2, 1, 1)
ax1.plot(raw_data, color='gray', alpha=0.2, label='Nyers adat')
ax1.plot(filtered, color='blue', linewidth=1.5, label=f'Szűrt jel (EMA)')
for idx in events_idx:
    ax1.plot(idx, filtered[idx], 'ro', markersize=4) # Piros pötty az eseményeknél
ax1.set_title(f"Adaptív detektálás: {FILE_TO_CHECK}")
ax1.legend()
ax1.grid(True, alpha=0.3)

# Alsó grafikon: Dinamikus küszöb és zaj
ax2 = plt.subplot(2, 1, 2, sharex=ax1)
ax2.plot(threshold_history, color='red', label='Dinamikus küszöb (Threshold)')
ax2.fill_between(range(len(noise_history)), 0, noise_history * K, color='orange', alpha=0.2, label='Zaj komponens')
ax2.axhline(y=MIN_THRESHOLD, color='black', linestyle='--', alpha=0.5, label='Min Threshold (Padló)')
ax2.set_ylabel("Érték")
ax2.legend()
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.show()