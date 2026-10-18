"""
Continuously print all received CAN messages and send a random
CAN message with ID 0x123 every 200 ms.

Usage: Paste into the Script window and click Run while a measurement is active.
Press Stop to end the script.
"""
import cangaroo
import os
import time

# ---- configuration ----
TX_ID = 0x123
TX_DLC = 8
TX_INTERVAL = 0.2  # seconds
INTERFACE_ID = 513          # change to match your setup

# Print available interfaces
for iface in cangaroo.interfaces():
    print(f"Interface {iface['id']}: {iface['name']}")

print("Waiting for messages...\n")

last_send = time.time()

while True:
    # receive() blocks up to timeout — use a short timeout so we can
    # send at the desired interval even when no messages arrive.
    messages = cangaroo.receive(timeout=0.02)

    for msg in messages:
        iface = cangaroo.interface_name(msg.interface_id)
        data_hex = msg.get_data().hex(' ')

        if msg.extended:
            id_str = f"0x{msg.id:08X}"
        else:
            id_str = f"0x{msg.id:03X}"

        flags = []
        if msg.fd:
            flags.append("FD")
        if msg.brs:
            flags.append("BRS")
        if msg.rtr:
            flags.append("RTR")
        flag_str = f" [{','.join(flags)}]" if flags else ""

        direction = "RX" if msg.is_rx else "TX"

        print(f"{msg.timestamp:.6f}  {iface}  {direction}  {id_str}  "
              f"[{msg.dlc}]{flag_str}  {data_hex}")

    # Send a random message every TX_INTERVAL
    now = time.time()
    if now - last_send >= TX_INTERVAL:
        tx = cangaroo.Message()
        tx.id = TX_ID
        tx.set_data(os.urandom(TX_DLC))
        cangaroo.send(tx, interface_id=INTERFACE_ID)
        print(f"TX  0x{tx.id:03X}  [{tx.dlc}]  {tx.get_data().hex(' ')}")
        last_send = now
