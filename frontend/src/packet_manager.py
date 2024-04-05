from collections.abc import Callable
from time import sleep
from struct import unpack, pack, error
import threading
import socket


class PacketType:
    NULL = 0
    SET_CLIENT_ID = 1
    SET_ROOM_ID = 2
    JOIN_ROOM = 3
    CREATE_ROOM = 4
    DRAW_COMMAND = 5
    CANVAS_PACKET = 6
    GUESS_PACKET = 7
    CORRECT_GUESS = 8
    SET_DRAWER = 9


class CanvasPacket:
    def __init__(self, old_x, old_y, new_x, new_y, color):
        self.old_x = old_x
        self.old_y = old_y
        self.new_x = new_x
        self.new_y = new_y
        self.color = color

    def create_packet(self):
        return pack("HHHHH", self.old_x, self.old_y, self.new_x, self.new_y, self.color)
    
    def parse_packet(packet: bytes):
        try:
            return CanvasPacket(unpack("H", packet[0:2])[0], unpack("H", packet[2:4])[0], unpack("H", packet[4:6])[0], unpack("H", packet[6:8])[0], unpack("H", packet[8:10])[0])
        except Exception as e:
            print(packet)
            raise e

    def __str__(self) -> str:
        return f"CanvasPacket: {self.old_x}, {self.old_y}, {self.new_x}, {self.new_y}, {self.color}"


class Packet:
    def __init__(self, packet_type, client_id, room_id, data, data_length=0):
        self.packet_type = packet_type
        self.client_id = client_id
        self.room_id = room_id
        self.data_length = len(data)
        if data_length != 0:
            self.data_length = data_length
        self.data = data

    def create_packet(self):
        # 16 bits for room id, 8 bits for client id, 8 bits for packet type, 32 bits for packet length
        packet = pack("HBBI", self.room_id, self.client_id,
                      self.packet_type, len(self.data))

        # Append the data to the packet
        packet += self.data

        return packet

    def parse_packet(packet):
        new_packet = Packet(0, 0, 0, b"")

        try:
            new_packet.room_id = unpack("H", packet[0:2])[0]
            new_packet.client_id = unpack("B", packet[2:3])[0]
            new_packet.packet_type = unpack("B", packet[3:4])[0]
            new_packet.data_length = unpack("I", packet[4:8])[0]
        except error as e:
            print(packet)
            raise e

        return new_packet

    def __str__(self):
        return f"Packet: {self.packet_type}, {self.client_id}, {self.room_id}, {self.data_length}, {self.data}"


class ScribbleSocket:
    def __init__(self, address: str, port: int, on_packet_received: Callable[[Packet], None] = None):
        self.client_id = None
        self.room_id = None
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((address, port))
        self.socket.settimeout(5)

        # Read the header packet
        packet = self.socket.recv(8)

        print(Packet.parse_packet(packet))

        while Packet.parse_packet(packet).packet_type != PacketType.SET_CLIENT_ID:
            packet = self.socket.recv(8)

        self.client_id = Packet.parse_packet(packet).client_id
        print(f"Received client ID: {self.client_id}")

        # Start the reader thread
        self.reader_thread = ReaderThread(self, on_packet_received)
        self.reader_thread.start()

    def send_packet(self, packet: Packet):
        self.socket.send(packet.create_packet())

    def close(self):
        self.reader_thread.stop()
        sleep(0.1)
        
        # Wait for the reader thread to finish
        while self.reader_thread.fetching_data:
            print("Waiting for reader thread to finish...")
        self.socket.close()


class ReaderThread(threading.Thread):
    def __init__(self, scribble_socket: ScribbleSocket, on_packet_received: Callable[[Packet], None] = None):
        threading.Thread.__init__(self)
        self.scribble_socket = scribble_socket
        self.on_packet_received = on_packet_received
        self.running = True
        self.fetching_data = False

    def run(self):
        self.scribble_socket.socket.settimeout(0)
        self.scribble_socket.socket.setblocking(0)

        while self.running:
            try:
                packet = self.scribble_socket.socket.recv(8)

                if packet:
                    temp = Packet.parse_packet(packet)
                    
                    if temp.packet_type == PacketType.SET_DRAWER or temp.packet_type == PacketType.CANVAS_PACKET or temp.packet_type == PacketType.DRAW_COMMAND or temp.packet_type == PacketType.GUESS_PACKET:
                        self.scribble_socket.socket.setblocking(self.running)
                        self.fetching_data = True
                        packet = self.scribble_socket.socket.recv(temp.data_length)
                        temp.data = packet
                        self.fetching_data = False
                        self.scribble_socket.socket.setblocking(0)

                    if self.on_packet_received:
                        self.on_packet_received(temp)
            except BlockingIOError:
                pass

    def stop(self):
        self.running = False
