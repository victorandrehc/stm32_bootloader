import struct
import serial
import crcmod

crc16_ccitt = crcmod.predefined.mkCrcFun('ccitt-false')

class FrameProcessor(object):
   
    SOF = 0xA5
    VER = 0x01

    CMD_UNKNOWN     = 0
    CMD_PING        = 0x01
    CMD_START       = 0x02
    CMD_DATA        = 0x03
    CMD_END         = 0x04
    CMD_RESET       = 0x05
    CMD_ACK         = 0x7F
    CMD_NACK        = 0x7E

    ser = None

    def __init__(self, ser):
        self.ser = ser

    def send_frame(self, cmd, payload=b''):
        length = len(payload)

        # Header: VER (1) | CMD (1) | LEN_L (1) | LEN_H (1)
        header = struct.pack('<BBH', self.VER, cmd, length)  # exactly 4 bytes

        # # CRC16 over header + payload
        crc = crc16_ccitt(header + payload)

        # Build final frame: SOF + header + payload + CRC16
        frame = struct.pack('B', self.SOF) + header + payload + struct.pack('<H', crc)
        # frame = struct.pack('B', SOF) + header;

        # Send over UART
        self.ser.write(frame)
        self.ser.flush()  # ensures all bytes are sent

        print(f"frame: {frame} crc: {struct.pack('<H', crc)}")


    def recv_exact(self, length):
        data = b''
        while len(data) < length:
            chunk = self.ser.read(length - len(data))
            if not chunk:
                raise FrameError("UART timeout")
            data += chunk
        return data

    def recv_frame(self):
        # --- Find SOF ---
        while True:
            b = self.ser.read(1)
            if not b:
                raise FrameError("Timeout waiting for SOF")
            if b[0] == self.SOF:
                break

        # --- Read header ---
        # VER (1) | CMD (1) | LEN_L (1) | LEN_H (1)
        hdr = self.recv_exact(4)
        ver, cmd, len_l, len_h = struct.unpack('<BBBB', hdr)
        length = len_l | (len_h << 8)

        if ver != self.VER:
            raise FrameError(f"Unsupported protocol version {ver}")

        # --- Read payload ---
        payload = self.recv_exact(length)

        # --- Read CRC ---
        crc_rx_bytes = self.recv_exact(2)
        crc_rx = struct.unpack('<H', crc_rx_bytes)[0]

        # --- CRC check ---
        crc_data = hdr + payload
        crc_calc = crc16_ccitt(crc_data)

        if crc_calc != crc_rx:
            raise FrameError(
                f"CRC mismatch: calc=0x{crc_calc:04X}, rx=0x{crc_rx:04X}"
            )

        return cmd, payload
    
    def recv_ack_nack(self):
        cmd,payload = self.recv_frame()
        return cmd == self.CMD_ACK