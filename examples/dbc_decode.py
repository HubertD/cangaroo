"""
Decode received CAN messages using loaded DBC files.

Demonstrates:
  - cangaroo.databases()  — list all loaded DBC files and their messages
  - cangaroo.decode(msg)  — decode a message into physical signal values
  - cangaroo.lookup(msg)  — inspect the DBC definition (bit layout, scaling, etc.)

Usage: Load one or more DBC files in the Measurement Setup, start the
measurement, then run this script.
"""
import cangaroo

# ---- show loaded databases ----
dbs = cangaroo.databases()
if not dbs:
    print("No DBC files loaded. Add a DBC in Measurement Setup.")
else:
    for db in dbs:
        print(f"Database: {db['file']}  (network: {db['network']})")
        for m in db['messages']:
            sig_names = ', '.join(m['signals']) if m['signals'] else '(none)'
            print(f"  0x{m['id']:03X}  {m['name']}  [{m['dlc']}]  {sig_names}")
    print()

# ---- receive and decode ----
print("Waiting for messages...\n")

while True:
    messages = cangaroo.receive(timeout=1.0)

    for msg in messages:
        decoded = cangaroo.decode(msg)

        if decoded is None:
            # No DBC definition for this ID — print raw
            print(f"0x{msg.id:03X}  [{msg.dlc}]  {msg.get_data().hex(' ')}  (unknown)")
            continue

        sender = decoded.get('sender', '?')
        print(f"0x{msg.id:03X}  {decoded['message']}  (sender: {sender})")

        for name, sig in decoded['signals'].items():
            value_name = sig.get('value_name', '')
            if value_name:
                print(f"  {name}: {value_name} ({sig['value']:.4g} {sig['unit']})")
            else:
                print(f"  {name}: {sig['value']:.4g} {sig['unit']}"
                      f"  [raw: {sig['raw']}]")
