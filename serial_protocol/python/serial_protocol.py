import struct
import serial
import crcmod


import struct
import crcmod

crc16_ccitt = crcmod.predefined.mkCrcFun('ccitt-false')

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

def send_frame(ser, cmd, payload=b''):
    length = len(payload)

    # Header: VER (1) | CMD (1) | LEN_L (1) | LEN_H (1)
    header = struct.pack('<BBH', VER, cmd, length)  # exactly 4 bytes

    # # CRC16 over header + payload
    crc = crc16_ccitt(header + payload)

    # Build final frame: SOF + header + payload + CRC16
    frame = struct.pack('B', SOF) + header + payload + struct.pack('<H', crc)
    # frame = struct.pack('B', SOF) + header;

    # Send over UART
    ser.write(frame)
    ser.flush()  # ensures all bytes are sent

    print(f"frame: {frame} crc: {struct.pack('<H', crc)}")


def recv_exact(ser, length):
    data = b''
    while len(data) < length:
        chunk = ser.read(length - len(data))
        if not chunk:
            raise FrameError("UART timeout")
        data += chunk
    return data

def recv_frame(ser):
    # --- Find SOF ---
    while True:
        b = ser.read(1)
        if not b:
            raise FrameError("Timeout waiting for SOF")
        if b[0] == SOF:
            break

    # --- Read header ---
    # VER (1) | CMD (1) | LEN_L (1) | LEN_H (1)
    hdr = recv_exact(ser, 4)
    ver, cmd, len_l, len_h = struct.unpack('<BBBB', hdr)
    length = len_l | (len_h << 8)

    if ver != VER:
        raise FrameError(f"Unsupported protocol version {ver}")

    # --- Read payload ---
    payload = recv_exact(ser, length)

    # --- Read CRC ---
    crc_rx_bytes = recv_exact(ser, 2)
    crc_rx = struct.unpack('<H', crc_rx_bytes)[0]

    # --- CRC check ---
    crc_data = hdr + payload
    crc_calc = crc16_ccitt(crc_data)

    if crc_calc != crc_rx:
        raise FrameError(
            f"CRC mismatch: calc=0x{crc_calc:04X}, rx=0x{crc_rx:04X}"
        )

    return cmd, payload

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)

send_frame(ser, CMD_PING)  # PING

cmd,payload = recv_frame(ser)
if(cmd != CMD_ACK):
    printf("error")
    exit(-1)
else:
    print("ACK RECEIVED")


send_frame(ser, CMD_START,bytearray([11,0]))    
# ser.write(b'HELLO WORLD')
