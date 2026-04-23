import serial
import time

# --- BEÁLLÍTÁSOK ---
# Cseréld ki a 'COM3'-at arra, amit az Arduino IDE-ben látsz!
# Linuxon/Mac-en valami ilyesmi: '/dev/ttyUSB0' vagy '/dev/tty.usbmodem...'
PORT = 'COM7' 
BAUD_RATE = 115200
FILE_NAME = "feny_meres.csv"

try:
    # Kapcsolat megnyitása
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    time.sleep(2) # Várunk, amíg az Arduino újraindul a kapcsolódáskor
    
    print(f"Kapcsolódva: {PORT}")
    print(f"Mentés folyamatban a '{FILE_NAME}' fájlba...")
    print("Megállításhoz nyomj CTRL+C-t!")

    # Fájl megnyitása (ha van már ilyen, a végéhez írja)
    with open(FILE_NAME, "a", encoding="utf-8") as file:
        while True:
            if ser.in_waiting > 0:
                # Beolvasunk egy sort, dekódoljuk, és levágjuk a felesleges entereket
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if line:
                    # Mentés és kiírás a képernyőre
                    file.write(line + "\n")
                    file.flush() # Azonnali írás a fájlba
                    print(line)

except serial.SerialException:
    print(f"Hiba: Nem sikerült elérni a {PORT} portot!")
except KeyboardInterrupt:
    print("\nMentés vége (CTRL+C megnyomva).")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()