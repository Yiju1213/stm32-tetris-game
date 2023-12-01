import serial
import keyboard
import time
import numpy as np

class KeyPacket:
    def __init__(self) -> None:
        # 报文长度
        self.len = 4
        # 报文具体内容
        self.head = np.uint8(0xF0)
        self.data = np.uint8()
        self.crc = np.uint8()
        self.tail = np.uint8(0x0F)
    def compute_crc(self) -> np.uint8 :
        return 0x00
    def to_array(self) -> np.ndarray:
        return np.array([self.head, self.data, self.crc, self.tail], dtype=np.uint8)

class TetrisController:
            
    def __init__(self, port, baudrate=115200, timeout=1):
        # 登记按键按下事件
        self.register_key_event()
        # 尝试连接
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.continuous_reconnect(1)
        time.sleep(0.5)
    
    def register_key_event(self):
        keyboard.on_release_key('up arrow', lambda event: self.send_command_packet(event))
        keyboard.on_release_key('down arrow', lambda event: self.send_command_packet(event))
        keyboard.on_release_key('left arrow', lambda event: self.send_command_packet(event))
        keyboard.on_release_key('right arrow', lambda event: self.send_command_packet(event))
        keyboard.on_release_key('space', lambda event: self.send_command_packet(event))

    def send_command_packet(self, key:keyboard.KeyboardEvent):
        # print(f"send key: {key.name} value: {key.scan_code}")

        packet = KeyPacket()
        packet.data = (np.uint8)(key.scan_code)
        packet.crc = packet.compute_crc()

        # print(f"{packet.head} {packet.data} {packet.crc} {packet.tail}")
        try:
            self.serial_port.write(packet.to_array())
        except serial.SerialException as e:
            print(f"{e}")
            self.close()
            self.continuous_reconnect(1)
        
    # 测试用 不用管
    def recv(self):
        packet = KeyPacket()
        try:
            num = self.serial_port.in_waiting
        except:
            num = 0
            self.close()
        if num > 0:
            try:
                # 读取起始帧 读不到就返回
                while self.serial_port.read() != bytes([packet.head]):
                    num -= 1
                    if num <= 0:
                        raise ValueError("Error: no head byte recv")
                print(f"read head")
                packet.data = np.frombuffer(self.serial_port.read(), dtype=np.uint8)
                print(f"read data {packet.data}")
                packet.crc = np.frombuffer(self.serial_port.read(), dtype=np.uint8)
                print(f"read crc")
                if self.serial_port.read() != bytes([packet.tail]):
                    raise ValueError("Error: wrong tail byte recv")
                print(f"read tail")
            except ValueError as e:
                print(e)

    def close(self):
        print("Tetris Controller stopped.")
        self.serial_port.close()

    def continuous_reconnect(self, retry_interval=2):
        while True:
            try:
                print(f"Connecting to {self.port}...")
                self.serial_port = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
                print("Tetris Controller is running. \
                      \n--------------------------\
                      \n|  Press 'q' to quit.    |\
                      \n|  Press '←' to go left  |\
                      \n|  Press '↓' to speed up |\
                      \n|  Press '→' to go right |\
                      \n|  Press 'space' to spin |\
                      \n--------------------------")
                break  # 连接成功，退出循环
            except serial.SerialException as e:
                print(f"Connection attempt failed: {e}")
                time.sleep(retry_interval)

    def run(self):
        try:
            while True:
                # 尝试发一个无关紧要的字节测试连接
                key = keyboard.KeyboardEvent('up', 72)
                self.send_command_packet(key)
                # 控制测试频率
                time.sleep(1)  
        except KeyboardInterrupt:
            pass
        finally:
            self.close()
            

# 在这里替换串口名称，例如'/dev/ttyUSB0'或'COM3'
serial_port_name = 'COM4'

# 创建并运行TetrisController
controller = TetrisController(serial_port_name)
controller.run()
while True:
    time.sleep(1)  
