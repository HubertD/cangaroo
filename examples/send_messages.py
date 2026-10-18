"""
Send CAN messages on a given interface.

Usage: Paste into the Script window and click Run while a measurement is active.
Adjust interface_id to match your setup (see cangaroo.interfaces()).
"""
import cangaroo
import time

# Print available interfaces
for iface in cangaroo.interfaces():
    print(f"Interface {iface['id']}: {iface['name']}")

# --- Send a single standard CAN message ---
msg = cangaroo.Message()
msg.id = 0x123
msg.set_data(bytes([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08]))
cangaroo.send(msg, interface_id=0)
print(f"Sent: ID=0x{msg.id:03X} DLC={msg.dlc} data={msg.get_data().hex(' ')}")

# --- Send an extended frame ---
msg2 = cangaroo.Message()
msg2.id = 0x18FEF100
msg2.extended = True
msg2.set_data(bytes([0xAA, 0xBB, 0xCC, 0xDD]))
cangaroo.send(msg2, interface_id=0)
print(f"Sent: ID=0x{msg2.id:08X} (ext) DLC={msg2.dlc} data={msg2.get_data().hex(' ')}")

# --- Send an RTR frame ---
msg3 = cangaroo.Message()
msg3.id = 0x200
msg3.rtr = True
msg3.dlc = 8
cangaroo.send(msg3, interface_id=0)
print(f"Sent: ID=0x{msg3.id:03X} RTR DLC={msg3.dlc}")

# --- Send periodic messages ---
print("\nSending 0x100 every 100ms (10 times)...")
periodic = cangaroo.Message()
periodic.id = 0x100
counter = 0

for i in range(10):
    periodic.set_data(bytes([counter & 0xFF, (counter >> 8) & 0xFF, 0, 0, 0, 0, 0, 0]))
    cangaroo.send(periodic, interface_id=0)
    print(f"  [{i+1}/10] counter={counter}")
    counter += 1
    time.sleep(0.1)

print("Done.")
