import json, time, csv, os
from datetime import datetime
import serial

# === EDIT THIS TO YOUR PORT ===
PORT = "COM3"          # e.g., "COM3" on Windows, "/dev/ttyACM0" on Linux, "/dev/tty.usbmodemXXXX" on macOS
BAUD = 115200

ASSET_ID = "A1000"
SERVICE_INTERVAL_HOURS = 800  # tweak later if you want

RAW_CSV = "live_sensor_log.csv"
DASH_CSV = "sensor_snapshot.csv"
STATE_FILE = "state_runtime.json"

def load_state():
    try:
        if os.path.exists(STATE_FILE):
            import json
            return json.load(open(STATE_FILE, "r"))
    except Exception:
        pass
    return {"last_service_hours": 0.0}

def save_state(state):
    import json
    json.dump(state, open(STATE_FILE, "w"))

state = load_state()

def init_csvs():
    if not os.path.exists(RAW_CSV):
        with open(RAW_CSV, "w", newline="") as f:
            csv.writer(f).writerow([
                "asset_id","timestamp_iso","temp_c","ambient_c","humidity",
                "vibration_events","equipment_on","runtime_minutes"
            ])
    if not os.path.exists(DASH_CSV):
        with open(DASH_CSV, "w", newline="") as f:
            csv.writer(f).writerow([
                "AssetID","Timestamp","Temp_C","Vibration_Events",
                "Equipment_On","Runtime_Minutes","Hours_Since_LastService"
            ])

init_csvs()

print(f"Opening {PORT} @ {BAUD}… (Ctrl+C to stop)")
ser = serial.Serial(PORT, BAUD, timeout=2)

def append_rows(raw_row, dash_row):
    with open(RAW_CSV, "a", newline="") as f:
        csv.writer(f).writerow(raw_row)
    with open(DASH_CSV, "a", newline="") as f:
        csv.writer(f).writerow(dash_row)

try:
    while True:
        line = ser.readline().decode("utf-8", errors="ignore").strip()
        if not line:
            continue
        try:
            msg = json.loads(line)
        except Exception:
            continue

        ts = datetime.utcnow().isoformat(timespec="seconds") + "Z"
        temp_c = float(msg.get("temp_c", "nan"))
        ambient_c = float(msg.get("ambient_c", "nan"))
        humidity = float(msg.get("humidity", "nan"))
        vib = int(msg.get("vibration_events", 0))
        equip_on = bool(msg.get("equipment_on", False))
        runtime_min = float(msg.get("runtime_minutes", 0.0))

        runtime_hours = runtime_min / 60.0
        hours_since_last_service = max(0.0, runtime_hours - load_state()["last_service_hours"])

        raw_row = [ASSET_ID, ts, temp_c, ambient_c, humidity, vib, equip_on, runtime_min]
        dash_row = [ASSET_ID, ts, temp_c, vib, equip_on, runtime_min, round(hours_since_last_service, 2)]

        append_rows(raw_row, dash_row)

        # Optional auto-reset when interval is reached
        if hours_since_last_service >= SERVICE_INTERVAL_HOURS:
            state = load_state()
            state["last_service_hours"] = runtime_hours
            save_state(state)
            print("** Auto service reset (interval reached) **")

        print(f"{ts}  T={temp_c:.1f}C  V={vib}  ON={equip_on}  Rt={runtime_min:.1f}m  HrsSince={hours_since_last_service:.1f}")

except KeyboardInterrupt:
    print("\nStopping.")
finally:
    try:
        ser.close()
    except Exception:
        pass
