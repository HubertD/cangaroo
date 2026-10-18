"""
Send periodic CAN messages using send_periodic() / stop_periodic().

Each periodic task runs on its own background thread inside CANgaroo, so the
script can wait for incoming messages at the same time without any manual
timing loop.  All tasks are automatically stopped when the script stops.

Demonstrates:
  - cangaroo.send_periodic()  — start a repeating TX task, returns a handle
  - cangaroo.stop_periodic()  — cancel one task by handle
  - cangaroo.receive()        — wait for replies while periodic TX is running

Usage: Load demo.dbc in Measurement Setup, start the measurement, then run.
Adjust INTERFACE_ID and intervals to match your setup.
"""
import cangaroo
import time

INTERFACE_ID = 0   # change to match your setup

# ---- show available interfaces ----
for iface in cangaroo.interfaces():
    print(f"Interface {iface['id']}: {iface['name']}")

# ---- build messages to send periodically ----
heartbeat = cangaroo.Message()
heartbeat.id = 0x700
heartbeat.set_data(bytes([0x01]))

# Use encode() so we get the right DBC bit layout
engine_msg = cangaroo.encode("EngineData", {
    "EngineSpeed": 2000.0,
    "EngineTemp":  85.0,
    "OilPressure": 3.8,
})

# ---- start periodic tasks ----
h_heartbeat = cangaroo.send_periodic(heartbeat,   interval_ms=100,  interface_id=INTERFACE_ID)
h_engine    = cangaroo.send_periodic(engine_msg,  interval_ms=20,   interface_id=INTERFACE_ID)

print(f"Started heartbeat  (handle={h_heartbeat}, every 100 ms)")
print(f"Started EngineData (handle={h_engine},    every 20 ms)")
print("Listening for incoming messages for 3 seconds...\n")

# ---- receive loop while periodic TX runs in the background ----
deadline = time.time() + 3.0
while time.time() < deadline:
    msgs = cangaroo.receive(timeout=0.1)
    for msg in msgs:
        decoded = cangaroo.decode(msg)
        if decoded:
            print(f"RX  0x{msg.id:03X}  {decoded['message']}")
        else:
            print(f"RX  0x{msg.id:03X}  [{msg.dlc}]  {msg.get_data().hex(' ')}")

# ---- stop individual tasks ----
cangaroo.stop_periodic(h_engine)
print(f"\nStopped EngineData (handle={h_engine}).")
print("Heartbeat still running for 1 more second...")
time.sleep(1.0)

# h_heartbeat is stopped automatically when the script exits,
# but we can also stop it explicitly:
cangaroo.stop_periodic(h_heartbeat)
print(f"Stopped heartbeat (handle={h_heartbeat}).")
print("Done.")
