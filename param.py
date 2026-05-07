import pandas as pd
import numpy as np
import time

# --- 1. BEÁLLÍTÁSOK ---
FILES = {
    "zaj": "zaj.csv",
    "meres": "feny_meres.csv" 
}
DELIMITER = "|"
COL_NAMES = ['timestamp', 'w', 't', 'e', 'f', 'raw', 'total']

# Finomhangolt tartományok
alphas = [0.05, 0.1, 0.15, 0.2]
noise_alphas = [0.001, 0.002, 0.005]
K_values = np.linspace(1.1, 5.0, 20)       # Szélesebb tartomány
min_thresholds = np.linspace(0.5, 4.0, 10) # Alacsonyabbról indulunk

def simulate_fast(raw_data, alpha, n_alpha, K, min_t):
    if len(raw_data) < 100: return 0
    
    # INICIALIZÁLÁS: Az első adatponttal kezdünk, nem 0-val!
    curr_f = raw_data[0]
    last_p = raw_data[0]
    avg_n = 0.5
    ev = 0
    
    # 100 minta bemelegedés (hogy a szűrők beálljanak)
    for i in range(1, len(raw_data)):
        val = raw_data[i]
        
        # EMA szűrő (Jel)
        curr_f = (val * alpha) + (curr_f * (1.0 - alpha))
        
        # EMA szűrő (Zaj/Jitter)
        jitter = abs(val - curr_f)
        avg_n = (jitter * n_alpha) + (avg_n * (1.0 - n_alpha))
        
        # Adaptív küszöb képlete:
        # $$Threshold = (AvgNoise \cdot K) + MinT$$
        dt = (avg_n * K) + min_t
        if dt > 30: dt = 30
        
        # Detektálás: Csak a 100. minta után számolunk eseményt
        if i > 100:
            if abs(curr_f - last_p) >= dt:
                ev += 1
                last_p = curr_f
    return ev

# --- 2. ADATOK BETÖLTÉSE ---
data = {}
for name, path in FILES.items():
    try:
        data[name] = pd.read_csv(path, sep=DELIMITER, names=COL_NAMES, header=0)['raw'].values
    except:
        print(f"Hiba: {path} nem tölthető be!")

# --- 3. AUTOMATIZÁLT KERESÉS ---
results = []
test_files = [k for k in data.keys() if k != "zaj"]
start_time = time.time()

if "zaj" in data and test_files:
    print("Keresés indítása (Startup korrekcióval)...")
    
    for a in alphas:
        for na in noise_alphas:
            for k in K_values:
                for mt in min_thresholds:
                    
                    # Csak akkor megyünk tovább, ha a zaj.csv-n TÉNYLEG 0 az esemény
                    if simulate_fast(data["zaj"], a, na, k, mt) == 0:
                        total_ev = 0
                        all_seen = True
                        
                        for fname in test_files:
                            c = simulate_fast(data[fname], a, na, k, mt)
                            if c == 0:
                                all_seen = False
                                break
                            total_ev += c
                        
                        if all_seen:
                            results.append({
                                'alpha': a, 'na': na, 'K': k, 'mt': mt, 
                                'ev': total_ev, 
                                'score': total_ev / (k * mt) # Érzékenységre optimalizálva
                            })

    # --- 4. EREDMÉNYEK ---
    runtime = time.time() - start_time
    print(f"Kész! ({runtime:.2f} mp)")

    if not results:
        print("\n❌ Nincs találat.")
        print("OK: A zaj.csv-ben lévő 2100-as ugrás túl nagy. Próbáld meg a zaj.csv-t")
        print("rövidebbre vágni (csak a 2100. minta előtti részt hagyd meg)!")
    else:
        best = sorted(results, key=lambda x: x['score'], reverse=True)[0]
        print("\n" + "="*50)
        print("🏆 OPTIMÁLIS ARDUINO PARAMÉTEREK")
        print("="*50)
        print(f"const float filterWeight = {best['alpha']:.3f};")
        print(f"const float noiseWeight   = {best['na']:.4f};")
        print(f"const float SENSITIVITY   = {best['K']:.2f};")
        print(f"const float MIN_THRESHOLD = {best['mt']:.2f};")
        print("="*50)