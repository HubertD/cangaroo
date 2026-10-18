"""
Start / stop measurements and inspect the trace from a script.

Demonstrates:
  - cangaroo.measurement_running()  — check whether a measurement is active
  - cangaroo.start_measurement()    — start the measurement programmatically
  - cangaroo.stop_measurement()     — stop the measurement programmatically
  - cangaroo.trace_size()           — number of messages currently in the trace
  - cangaroo.clear_trace()          — reset the trace buffer
  - cangaroo.find_message()         — inspect a DBC message definition by name

Usage: Configure your interfaces in Measurement Setup (no need to start the
measurement manually), then run this script.
"""
import cangaroo
import time

# ---- check initial state ----
if cangaroo.measurement_running():
    print("Measurement is already running — stopping it first.")
    cangaroo.stop_measurement()

print(f"Measurement running: {cangaroo.measurement_running()}")
print(f"Trace size before start: {cangaroo.trace_size()} messages")

# ---- start measurement ----
ok = cangaroo.start_measurement()
print(f"\nstart_measurement() -> {ok}")
print(f"Measurement running: {cangaroo.measurement_running()}")

# ---- collect traffic for a few seconds ----
print("\nCollecting traffic for 3 seconds...")
time.sleep(3.0)
print(f"Trace size after 3 s: {cangaroo.trace_size()} messages")

# ---- inspect DBC definitions (no live message needed) ----
for name in ("EngineData", "TransmissionData", "AmbientData"):
    defn = cangaroo.find_message(name)
    if defn is None:
        print(f"\n{name}: not found (is demo.dbc loaded?)")
    else:
        sig_names = [s["name"] for s in defn["signals"]]
        print(f"\n{defn['message']}  ID=0x{defn['id']:03X}  DLC={defn['dlc']}")
        print(f"  Signals: {', '.join(sig_names)}")

# ---- clear trace and collect again ----
print("\nClearing trace...")
cangaroo.clear_trace()
print(f"Trace size after clear: {cangaroo.trace_size()} messages")

time.sleep(1.0)
print(f"Trace size after 1 more second: {cangaroo.trace_size()} messages")

# ---- stop measurement ----
ok = cangaroo.stop_measurement()
print(f"\nstop_measurement() -> {ok}")
print(f"Measurement running: {cangaroo.measurement_running()}")
print(f"Final trace size: {cangaroo.trace_size()} messages")

print("\nDone.")
