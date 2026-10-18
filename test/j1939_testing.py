import can
import time
import struct
import math

# --- Helper Function for DM1 Packing ---
def pack_dm1_payload(lamp_status, dtc_list):
    """
    Packs the DM1 message (22 bytes) based on the specific bit layout.
    """
    payload = bytearray([0xFF] * 22)

    # Byte 0: Lamp Statuses
    byte0 = 0x00
    byte0 |= (lamp_status.get('protect', 0) & 0x03) << 0
    byte0 |= (lamp_status.get('amber', 0) & 0x03) << 2
    byte0 |= (lamp_status.get('red', 0) & 0x03) << 4
    byte0 |= (lamp_status.get('malfunction', 0) & 0x03) << 6
    payload[0] = byte0

    # Byte 1: Flash Statuses
    byte1 = 0x00
    byte1 |= (lamp_status.get('flash_protect', 0) & 0x03) << 0
    byte1 |= (lamp_status.get('flash_amber', 0) & 0x03) << 2
    byte1 |= (lamp_status.get('flash_red', 0) & 0x03) << 4
    byte1 |= (lamp_status.get('flash_malfunction', 0) & 0x03) << 6
    payload[1] = byte1

    # Bytes 2-21: DTCs (5 DTCs, 4 bytes each)
    for i in range(5):
        if i < len(dtc_list):
            dtc_val = dtc_list[i]
        else:
            dtc_val = 0x00000000 
            
        packed_dtc = struct.pack('<I', dtc_val)
        start_byte = 2 + (i * 4)
        payload[start_byte : start_byte+4] = packed_dtc

    return payload


# --- Main Simulator Class ---
class J1939Simulator:
    def __init__(self, channel='vcan0', bustype='socketcan'):
        self.bus = can.interface.Bus(channel=channel, bustype=bustype)
        print(f"--- J1939 Simulator Started on {channel} ---")

    def _build_j1939_id(self, priority, pgn, source_addr):
        can_id = (priority << 26) | (pgn << 8) | source_addr
        return can_id

    def send_single_frame(self, priority, pgn, source_addr, data):
        if len(data) > 8:
            print(f"Error: Data length {len(data)} is too long for Single Frame.")
            return

        can_id = self._build_j1939_id(priority, pgn, source_addr)
        message = can.Message(arbitration_id=can_id, data=data, is_extended_id=True)
        
        self.bus.send(message)
        # Using simple print for cleaner periodic output
        print(f"[SF]   PGN: {pgn:04X} | SA: {source_addr:02X} | Data: {data.hex().upper()}")

    def send_multiframe_bam(self, priority, pgn, source_addr, data):
        """
        Sends Multi-frame BAM (TP.CM + TP.DTs)
        """
        # Constants
        TP_CM_PGN = 0x00EC00
        TP_DT_PGN = 0x00EB00
        BAM_CONTROL_BYTE = 32
        
        total_bytes = len(data)
        total_packets = math.ceil(total_bytes / 7)
        
        # 1. Send TP.CM
        pgn_bytes = struct.pack('<I', pgn)[0:3] 
        tp_cm_data = [
            BAM_CONTROL_BYTE,
            total_bytes & 0xFF,
            (total_bytes >> 8) & 0xFF,
            total_packets,
            0xFF,
            pgn_bytes[0],
            pgn_bytes[1],
            pgn_bytes[2]
        ]
        
        tp_cm_id = self._build_j1939_id(priority, TP_CM_PGN, source_addr)
        self.bus.send(can.Message(arbitration_id=tp_cm_id, data=tp_cm_data, is_extended_id=True))
        print(f"[BAM]  PGN: {pgn:04X} | Start (Size: {total_bytes}, Packets: {total_packets})")

        time.sleep(0.050) # 50ms standard delay

        # 2. Send TP.DT
        tp_dt_id = self._build_j1939_id(priority, TP_DT_PGN, source_addr)
        
        for i in range(total_packets):
            seq_num = i + 1
            chunk = data[i*7 : (i+1)*7]
            
            if len(chunk) < 7:
                chunk += b'\xFF' * (7 - len(chunk))
            
            dt_data = bytearray([seq_num]) + chunk
            self.bus.send(can.Message(arbitration_id=tp_dt_id, data=dt_data, is_extended_id=True))
            
            # Reduce print spam for periodic messages (optional)
            # print(f"       -> Seq: {seq_num} | {chunk.hex().upper()}")
            
            time.sleep(0.010) # 10ms inter-packet delay
            
        print(f"[BAM]  PGN: {pgn:04X} | Complete")


# --- Main Execution Loop ---
if __name__ == "__main__":
    try:
        sim = J1939Simulator(channel='vcan0')

        # --- Define Static Data ---
        # 1. EEC1 Data (Engine Speed etc.)
        eec1_data = b'\x12\x34\x56\x78\x9A\xBC\xDE\xF0'
        
        # 2. VIN Data
        vin_data = b'1M8GDM9A_KP042788'

        # 3. DM1 Configuration
        lamps = {'protect': 0, 'amber': 1, 'red': 0, 'malfunction': 1, 'flash_malfunction': 1}
        dtc_codes = [0x00032064, 0x000450FF] # Two active DTCs
        
        print("\nStarting Periodic Transmission (Every 2s)... Press Ctrl+C to stop.\n")

        while True:
            cycle_start = time.time()
            print("-" * 40)
            
            # --- Message 1: EEC1 (Single Frame) ---
            # PGN 61444 (F004), Priority 3, SA 0
            sim.send_single_frame(priority=3, pgn=0xF004, source_addr=0x00, data=eec1_data)
            
            # --- Message 2: VIN (Multi-Frame BAM) ---
            # PGN 65260 (FEEC), Priority 6, SA 0
            sim.send_multiframe_bam(priority=6, pgn=0xFEEC, source_addr=0x00, data=vin_data)
            
            # --- Message 3: DM1 (Multi-Frame BAM) ---
            # PGN 65226 (FECA), Priority 6, SA 72 (0x48)
            dm1_payload = pack_dm1_payload(lamps, dtc_codes)
            sim.send_multiframe_bam(priority=6, pgn=0xFECA, source_addr=0x48, data=dm1_payload)

            # Wait for remainder of 2 seconds
            elapsed = time.time() - cycle_start
            sleep_time = max(0, 2.0 - elapsed)
            time.sleep(sleep_time)

    except OSError:
        print("Error: Could not find vcan0. Run: sudo ip link add dev vcan0 type vcan && sudo ip link set up vcan0")
    except KeyboardInterrupt:
        print("\nSimulation stopped by user.")