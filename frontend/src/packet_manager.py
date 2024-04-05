from collections.abc import Callable
from struct import *
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
    NEW_ROUND = 8
    CORRECT_GUESS = 9
    INCORRECT_GUESS = 10
    SET_DRAWER = 11


class Packet:
    def __init__(self, packet_type, client_id, room_id, data):
        self.packet_type = packet_type
        self.client_id = client_id
        self.room_id = room_id
        self.data_length = len(data)
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

        new_packet.room_id = unpack("H", packet[0:2])[0]
        new_packet.client_id = unpack("B", packet[2:3])[0]
        new_packet.packet_type = unpack("B", packet[3:4])[0]
        new_packet.data_length = unpack("I", packet[4:8])[0]

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

        packet = self.socket.recv(64)

        print(Packet.parse_packet(packet))

        while Packet.parse_packet(packet).packet_type != PacketType.SET_CLIENT_ID:
            packet = self.socket.recv(64)

        self.client_id = Packet.parse_packet(packet).client_id
        print(f"Received client ID: {self.client_id}")

        # Start the reader thread
        self.reader_thread = ReaderThread(self, on_packet_received)
        self.reader_thread.start()

    def send_packet(self, packet: Packet):
        self.socket.send(packet.create_packet())

    def close(self):
        self.reader_thread.stop()
        # Wait for the reader thread to finish
        while self.reader_thread.fetching_data:
            pass
        self.socket.close()


class ReaderThread(threading.Thread):
    def __init__(self, scribble_socket: ScribbleSocket, on_packet_received: Callable[[Packet], None] = None):
        threading.Thread.__init__(self)
        self.scribble_socket = scribble_socket
        self.on_packet_received = on_packet_received
        self.running = True
        self.fetching_data = False

    def run(self):
        self.scribble_socket.socket.settimeout(None)
        self.scribble_socket.socket.setblocking(False)

        while self.running:
            try:
                packet = self.scribble_socket.socket.recv(64)

                if packet:
                    temp = Packet.parse_packet(packet)

                    if temp.packet_type == PacketType.SET_DRAWER or temp.packet_type == PacketType.CANVAS_PACKET:
                        self.scribble_socket.socket.setblocking(self.running)
                        self.fetching_data = True
                        packet = self.scribble_socket.socket.recv(
                            temp.data_length)
                        self.fetching_data = False
                        self.scribble_socket.socket.setblocking(False)
                        temp.data += packet

                    if self.on_packet_received:
                        self.on_packet_received(temp)
            except BlockingIOError:
                pass

    def stop(self):
        self.running = False
