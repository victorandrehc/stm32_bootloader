import struct
import serial
import crcmod

import serial_process_frame

from enum import Enum

class State(Enum):
    PING = 0
    START = 1
    DATA = 2
    END = 3
    RESET = 4
    DONE = 5



class FirmwareUpdater:
    def __init__(self, frame_processor,firmware: bytes, chunk_size=256):
        self.frame_processor = frame_processor
        self.fw = firmware
        self.chunk_size = chunk_size
        self.offset = 0
        self.state = State.PING

    def wait_ack(self):
        cmd, payload = self.frame_processor.recv_frame()
        if cmd == self.frame_processor.CMD_ACK:
            return True
        if cmd == self.frame_processor.CMD_NACK:
            raise RuntimeError("MCU NACK")
        raise RuntimeError(f"Unexpected CMD {cmd}")

    def run(self):
        while self.state != State.DONE:

            # ---- PING ----
            if self.state == State.PING:
                self.frame_processor.send_frame(self.frame_processor.CMD_PING)
                self.wait_ack()
                self.state = State.START

            # ---- START ----
            elif self.state == State.START:
                payload = struct.pack('<I', len(self.fw)) + struct.pack('<H', 0x0A0A)
                self.frame_processor.send_frame(self.frame_processor.CMD_START, payload)
                self.wait_ack()
                self.state = State.DATA

            # ---- DATA ----
            elif self.state == State.DATA:
                if self.offset >= len(self.fw):
                    self.state = State.END
                    continue

                chunk = self.fw[self.offset:self.offset + self.chunk_size]
                self.frame_processor.send_frame(self.frame_processor.CMD_DATA, chunk)
                self.wait_ack()
                self.offset += len(chunk)

                print(f"Progress: {self.offset}/{len(self.fw)}")

            # ---- END ----
            elif self.state == State.END:
                self.frame_processor.send_frame(self.frame_processor.CMD_END)
                self.wait_ack()
                self.state = State.RESET

            # ---- RESET ----
            elif self.state == State.RESET:
                self.state = State.DONE

        print("Firmware update complete")


if __name__ == "__main__":
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    frame_processor = serial_process_frame.FrameProcessor(ser)
    with open("firmware.bin", "rb") as f:
        firmware = f.read()
        updater = FirmwareUpdater(frame_processor, firmware)
        updater.run()

