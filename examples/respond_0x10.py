"""
React to CAN message 0x10 and send a response 0x321.

- Byte 0 of 0x321 is copied from Byte 0 of the received 0x10 message.
- Byte 1 of 0x321 contains an incrementing counter (wraps at 255).

Usage: Paste into the Script window and click Run while a measurement is active.
Adjust INTERFACE_ID to match your setup (see cangaroo.interfaces()).
"""
import cangaroo

TRIGGER_ID   = 0x010
RESPONSE_ID  = 0x321
INTERFACE_ID = 513        # change to match your setup

for iface in cangaroo.interfaces():
    print(f"Interface {iface['id']}: {iface['name']}")

counter = 0

def on_message(msg):
    global counter

    if msg.id != TRIGGER_ID:
        return

    data = msg.get_data()
    byte0 = data[0] if len(data) > 0 else 0x00

    resp = cangaroo.Message()
    resp.id = RESPONSE_ID
    resp.set_data(bytes([byte0, counter & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]))
    cangaroo.send(resp, interface_id=INTERFACE_ID)

    print(f"RX  0x{msg.id:03X}  [{msg.dlc}]  {data.hex(' ')}")
    print(f"TX  0x{resp.id:03X}  [{resp.dlc}]  byte0=0x{byte0:02X}  counter={counter}")

    counter = (counter + 1) & 0xFF


print(f"Listening for 0x{TRIGGER_ID:03X}, responding with 0x{RESPONSE_ID:03X}...\n")

while True:
    for msg in cangaroo.receive(timeout=1.0):
        on_message(msg)
