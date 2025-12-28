import struct
import serial
import crcmod

import serial_process_frame



ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
frame_processor = serial_process_frame.FrameProcessor(ser)

frame_processor.send_frame(frame_processor.CMD_PING)  # PING

if frame_processor.recv_ack_nack():
    print("ACK received")
else:
    print("NACK received")

frame_processor.send_frame(frame_processor.CMD_START,bytearray([11,0]))    
# ser.write(b'HELLO WORLD')
