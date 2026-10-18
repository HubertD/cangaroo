"""
Receive only selected CAN IDs using set_filter() / clear_filter().

The filter is applied inside CANgaroo before messages enter the receive()
queue, so filtered-out traffic never reaches the script.

Demonstrates:
  - cangaroo.set_filter()    — accept only messages whose (id & mask) == id
  - cangaroo.clear_filter()  — remove the filter, accept everything again
  - cangaroo.trace_size()    — total messages in the trace buffer
  - cangaroo.clear_trace()   — reset the trace buffer

Usage: Start the measurement, then run.  Works with or without a DBC loaded.
"""
import cangaroo
import time

# ---- helper ----
def receive_for(seconds, label):
    """Drain the receive queue for `seconds`, printing each message."""
    print(f"\n--- {label} ---")
    deadline = time.time() + seconds
    count = 0
    while time.time() < deadline:
        for msg in cangaroo.receive(timeout=0.1):
            decoded = cangaroo.decode(msg)
            if decoded:
                sig_summary = ", ".join(
                    f"{n}={v['value']:.4g}{v['unit']}"
                    for n, v in decoded["signals"].items()
                )
                print(f"  0x{msg.id:03X}  {decoded['message']}  {sig_summary}")
            else:
                print(f"  0x{msg.id:03X}  [{msg.dlc}]  {msg.get_data().hex(' ')}")
            count += 1
    print(f"  ({count} message(s) received)")


# ---- baseline: no filter ----
cangaroo.clear_trace()
receive_for(1.0, "No filter — all traffic")
print(f"Trace size after 1 s: {cangaroo.trace_size()} messages")

# ---- filter: accept only EngineData (ID 0x100 = 256) exactly ----
cangaroo.set_filter(0x100, mask=0x7FF)
receive_for(1.0, "Filter: ID 0x100 only (EngineData)")

# ---- filter: accept 0x100–0x1FF range (top 4 bits match 0x100) ----
cangaroo.set_filter(0x100, mask=0x700)
receive_for(1.0, "Filter: 0x100–0x1FF range")

# ---- filter: accept only extended frames ----
cangaroo.set_filter(0x00000000, mask=0x00000000, extended=True)
receive_for(1.0, "Filter: extended frames only")

# ---- remove filter ----
cangaroo.clear_filter()
cangaroo.clear_trace()
receive_for(1.0, "Filter cleared — all traffic again")
print(f"Trace size after clear + 1 s: {cangaroo.trace_size()} messages")

print("\nDone.")
