"""
Build and send CAN messages using DBC signal encoding.

Demonstrates:
  - cangaroo.encode()        — build a Message from named signal values
  - cangaroo.signal_value()  — extract a single signal's physical value
  - cangaroo.find_message()  — look up a DBC message definition by name or ID

Usage: Load demo.dbc in Measurement Setup, start the measurement, then run
this script.  Adjust interface_id to match your setup.
"""
import cangaroo
import time

INTERFACE_ID = 0   # change to match your setup

# ---- inspect a message definition before encoding ----
defn = cangaroo.find_message("EngineData")
if defn is None:
    print("EngineData not found — is demo.dbc loaded?")
else:
    print(f"Message: {defn['message']}  ID=0x{defn['id']:03X}  DLC={defn['dlc']}")
    for sig in defn["signals"]:
        print(f"  {sig['name']:20s}  [{sig['start_bit']}|{sig['length']}]"
              f"  factor={sig['factor']}  offset={sig['offset']}"
              f"  unit={sig['unit']!r}")
    print()

# ---- encode and send EngineData ----
engine_msg = cangaroo.encode("EngineData", {
    "EngineSpeed": 3500.0,   # rpm
    "EngineTemp":  90.0,     # degC
    "OilPressure": 4.2,      # bar
})
cangaroo.send(engine_msg, interface_id=INTERFACE_ID)
print(f"Sent EngineData:  {engine_msg}")

# Verify round-trip: decode the just-sent message
speed = cangaroo.signal_value(engine_msg, "EngineSpeed")
temp  = cangaroo.signal_value(engine_msg, "EngineTemp")
oil   = cangaroo.signal_value(engine_msg, "OilPressure")
print(f"  EngineSpeed={speed} rpm  EngineTemp={temp} degC  OilPressure={oil} bar")
print()

# ---- encode and send TransmissionData ----
# GearPos has value names: 0=Neutral 1=First … 7=Reverse
for gear_pos, speed_kmh in [(1, 15.0), (2, 35.0), (3, 60.0), (0, 0.0)]:
    tx = cangaroo.encode("TransmissionData", {
        "GearPos":      float(gear_pos),
        "VehicleSpeed": speed_kmh,
    })
    cangaroo.send(tx, interface_id=INTERFACE_ID)
    decoded = cangaroo.decode(tx)
    gear_name = decoded["signals"]["GearPos"].get("value_name", str(gear_pos))
    print(f"Sent TransmissionData: gear={gear_name}  speed={speed_kmh} km/h  {tx}")
    time.sleep(0.05)

print()

# ---- encode by raw ID instead of name ----
ambient_msg = cangaroo.encode(768, {   # 0x300 = AmbientData
    "OutsideTemp": 21.5,   # degC
    "Humidity":    65.0,   # %
})
cangaroo.send(ambient_msg, interface_id=INTERFACE_ID)
print(f"Sent AmbientData (by ID):  {ambient_msg}")
print(f"  OutsideTemp={cangaroo.signal_value(ambient_msg, 'OutsideTemp')} degC"
      f"  Humidity={cangaroo.signal_value(ambient_msg, 'Humidity')} %")

print("\nDone.")

# ── AUTOSAR E2E Profile 2 ──────────────────────────────────────────────────────
# e2e_p2_protect(msg, data_id, counter)
#   Writes the counter nibble into byte 1 and the CRC-8H2F into byte 0 in-place.
#
# e2e_p2_compute_crc(msg, data_id) -> int
#   Returns the CRC byte only; useful when you need to inspect or place it manually.
#
# Message layout after protect():
#   Byte 0: CRC (all 8 bits)
#   Byte 1: bits 7:4 = reserved (0), bits 3:0 = counter (0–14)
#   Byte 2+: payload

DATA_ID = 0x1234
counter = 0

# Build the message from DBC signals (bytes 0 and 1 are reserved for E2E header)
e2e_msg = cangaroo.encode("EngineData", {
    "EngineSpeed": 3500.0,
    "EngineTemp":  90.0,
})

# One-shot: write counter + CRC and send
cangaroo.e2e_p2_protect(e2e_msg, data_id=DATA_ID, counter=counter)
cangaroo.send(e2e_msg, interface_id=INTERFACE_ID)
print(f"Sent E2E-protected EngineData: CRC=0x{e2e_msg.get_byte(0):02X}  "
      f"counter nibble=0x{e2e_msg.get_byte(1):02X}  {e2e_msg}")

# Fine-grained: compute CRC manually and place it yourself
e2e_msg2 = cangaroo.encode("EngineData", {"EngineSpeed": 1000.0, "EngineTemp": 25.0})
e2e_msg2.set_byte(1, counter & 0x0F)                                   # write counter nibble first
crc = cangaroo.e2e_p2_compute_crc(e2e_msg2, data_id=DATA_ID)           # compute CRC
cangaroo.log(f"E2E P2 CRC: 0x{crc:02X}")
e2e_msg2.set_byte(0, crc)                                              # place CRC
cangaroo.send(e2e_msg2, interface_id=INTERFACE_ID)
