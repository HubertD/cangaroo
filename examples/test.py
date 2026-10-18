"""
Send 0x10 with a sine-wave on Byte 0 every 100 ms, and respond to every
received 0x10 with 0x321 (Byte 0 echoed, Byte 1 = counter).

TX interface (0x10):  interface_id = 256
RX/TX interface (0x321): interface_id = 513
"""
import cangaroo
import time
import math
import threading

TX_ID        = 0x010
RESPONSE_ID  = 0x321
TX_IFACE     = 0
RX_IFACE     = 512

for iface in cangaroo.interfaces():
    print(f"Interface {iface['id']}: {iface['name']}")

# --- sine-wave sender thread ---
samples = 256
sine_values = [int((math.sin(2 * math.pi * i / samples) + 1) * 127.5) for i in range(samples)]

def sender():
    msg = cangaroo.Message()
    msg.id = TX_ID
    i = 0
    while True:
        msg.set_data(bytes([sine_values[i % samples]]))
        cangaroo.send(msg, interface_id=TX_IFACE)
        print(f"TX  0x{TX_ID:03X}  byte0={sine_values[i % samples]}")
        i += 1
        time.sleep(0.1)

threading.Thread(target=sender, daemon=True).start()

# --- response loop ---
rx_counter = 0

print(f"Listening for 0x{TX_ID:03X}, responding with 0x{RESPONSE_ID:03X}...\n")

while True:
    for msg in cangaroo.receive(timeout=1.0):
        if msg.id != TX_ID or not msg.is_rx:
            continue

        data = msg.get_data()
        byte0 = data[0] if len(data) > 0 else 0x00

        resp = cangaroo.Message()
        resp.id = RESPONSE_ID
        resp.set_data(bytes([byte0, rx_counter & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]))
        cangaroo.send(resp, interface_id=RX_IFACE)

        print(f"RX  0x{msg.id:03X}  [{msg.dlc}]  {data.hex(' ')}")
        print(f"TX  0x{resp.id:03X}  [{resp.dlc}]  byte0=0x{byte0:02X}  counter={rx_counter}")

        rx_counter = (rx_counter + 1) & 0xFF
