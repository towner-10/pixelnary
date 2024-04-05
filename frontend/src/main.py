"""Handles main frontend functionality"""

import tkinter as tk
import screen_manager as sm
from packet_manager import *
from paint import *
import sys


class ScribbleGame:
    def __init__(self):
        self.socket: ScribbleSocket = None
        self.root: tk.Tk = sm.screen_manager.get_root()
        self.drawer = False

    def on_packet_received(self, packet: Packet):
        print(f"Received packet: {packet.packet_type}")

        if packet.packet_type == PacketType.SET_ROOM_ID:
            if self.socket is not None and self.socket.client_id is not None:
                print(f"Received room ID: {packet.room_id}")
                self.socket.room_id = packet.room_id
                self.socket.send_packet(Packet(PacketType.JOIN_ROOM, self.socket.client_id, packet.room_id, b""))
        elif packet.packet_type == PacketType.SET_DRAWER:
            self.drawer = int.from_bytes(packet.data) == 1
            print(self.drawer)
            self.render_room()
                

    def render_main_menu(self):
        sm.ScreenManager.get_instance().clear_screen()

        heading = sm.ScreenManager.get_instance().get_heading_font()
        text = sm.ScreenManager.get_instance().get_text_font()
        text_bold = (text[0], text[1], "bold")

        # Game title
        title_label = tk.Label(self.root, text="Pixelnary",
                            font=heading, bg="#2133AB", fg="white")
        title_label.place(x=600, y=260, anchor="center")

        # Subtitle
        subtitle_label = tk.Label(
            self.root, text="SE 3313B Stretch Project", font=text_bold, bg="#2133AB", fg="white")
        subtitle_label.place(x=600, y=325, anchor="center")

        # Server IP
        ip_label = tk.Label(self.root, text="Server IP:", font=text,
                            bg="#2133AB", fg="white")
        ip_label.place(x=600, y=400, anchor="center")

        # Server IP input
        ip_entry = tk.Entry(self.root, font=text, width=28)

        # Set default IP
        ip_entry.insert(0, "127.0.0.1:25565")

        # Render IP entry
        ip_entry.place(x=600, y=425, anchor="center")

        # Room ID join input
        input_entry = tk.Entry(self.root, font=text, width=16)
        input_entry.place(x=470, y=500)

        # Join button
        join_button = tk.Button(self.root, text="Join room",
                                font=text_bold, bg="#2EBF53", fg="white", width=10, height=1, command=lambda: self.handle_join(ip_entry.get(), input_entry.get()))
        join_button.place(x=675, y=510, anchor="center")

        # Create button
        create_button = tk.Button(self.root, text="Create a new room",
                                font=text_bold, bg="#288DD9", fg="white", width=25, height=2, command=lambda: self.handle_create(ip_entry.get()))
        create_button.place(x=600, y=575, anchor="center")

    def render_room(self):
        sm.ScreenManager.get_instance().clear_screen()

        subheading = sm.ScreenManager.get_instance().get_subheading_font()
        subheading_bold = (subheading[0], subheading[1], "bold")
        text = sm.ScreenManager.get_instance().get_text_font()
        text_bold = (text[0], text[1], "bold")

        # Leave button
        leave_button = tk.Button(self.root, text="Leave",
                                font=text_bold, bg="#288DD9", fg="white", width=6, height=1, command=lambda: self.handle_leave())
        leave_button.place(x=50, y=62)

        # Room ID title
        room_label = tk.Label(self.root, text="Room",
                            font=subheading_bold, bg="#2133AB", fg="white")
        room_label.place(x=165, y=50)
        id_label = tk.Label(self.root, text=self.socket.room_id,
                            font=subheading, bg="#2133AB", fg="white")
        id_label.place(x=295, y=50)

        canvas = Paint(self.root)
        canvas.setup(self.socket.room_id, self.socket.client_id, self.drawer)

        # Action label
        if (self.drawer):
            action = "Your word is test. Draw it!"
        else:
            action = "Guess the word!"

        action_label = tk.Label(self.root, text=action,
                                font=text_bold, bg="#2133AB", fg="white")
        action_label.place(x=950, y=160, anchor="center")

        # Guess chat box
        box = tk.Canvas(self.root, width=400, height=550, bg="#16208F")
        box.place(x=750, y=200)

        # Message input
        msg_entry = tk.Entry(self.root, font=text, width=34)
        msg_entry.place(x=762, y=720)

        user = "Client " + str(self.socket.client_id)
        msg_list = []  # TODO: Replace with real message list
        # Replace with not drawer (for now still enabled to show chat box)
        is_guessing = True
        components = []

        # Guess button
        guess_button = tk.Button(self.root, text="Guess",
                                font=text_bold, bg="#288DD9", fg="white", width=6, height=1,
                                command=lambda: handle_send(user, msg_entry, msg_list, is_guessing, components, "test"))
        guess_button.place(x=1109, y=729, anchor="center")

        # Display guess in chat box
        def handle_send(user, msg_entry, msg_list, is_guessing, components, word):
            # Prevent guess if user already got the right word or guess is empty
            if is_guessing == False or msg_entry.get() == "":
                return

            # Destroy current guess message components
            for component in components:
                component.destroy()

            # Add new guess message to msg_list
            msg_list.append({
                "user": user,
                "msg": msg_entry.get()
            })

            # Limit msg_list to 7 most recent messages
            if len(msg_list) > 7:
                msg_list.pop(0)

            # Clear message input
            msg_entry.delete(0, "end")

            # Render new guess message components in order of least-most recent
            for i in range(len(msg_list)):
                msg_box = tk.Canvas(self.root, width=380, height=60, bg="#16208F")
                msg_box.place(x=760, y=210 + 73 * i)
                components.append(msg_box)

                if (msg_list[i]["msg"] == word):
                    user_label = tk.Label(
                        self.root, text=f"{msg_list[i]['user']} guessed the right word!", font=text_bold, bg="#16208F", fg="#2EBF53")
                    user_label.place(x=770, y=220 + 73 * i)
                    components.append(user_label)

                    msg_label = tk.Label(
                        self.root, text="Congratulations!", font=text, bg="#16208F", fg="#2EBF53")
                    msg_label.place(x=770, y=240 + 73 * i)
                    components.append(msg_label)

                    is_guessing = False
                else:
                    user_label = tk.Label(
                        self.root, text=f"{msg_list[i]['user']} guessed:", font=text, bg="#16208F", fg="white")
                    user_label.place(x=770, y=220 + 73 * i)
                    components.append(user_label)

                    msg_label = tk.Label(
                        self.root, text=msg_list[i]["msg"], font=text_bold, bg="#16208F", fg="white")
                    msg_label.place(x=770, y=240 + 73 * i)
                    components.append(msg_label)

    def handle_join(self, ip: str, room_id: str):
        address = ip.split(":")[0]
        port = int(ip.split(":")[1])

        if self.socket is None:
            self.socket = ScribbleSocket(address, port, self.on_packet_received)

        # Send join packet
        self.socket.send_packet(Packet(PacketType.JOIN_ROOM,
                        self.socket.client_id, int(room_id), b""))
        
        self.socket.room_id = int(room_id)
        
    def handle_create(self, ip: str):
        address = ip.split(":")[0]
        port = int(ip.split(":")[1])

        if self.socket is None:
            self.socket = ScribbleSocket(address, port, self.on_packet_received)

        # Send create packet
        self.socket.send_packet(Packet(PacketType.CREATE_ROOM,
                        self.socket.client_id, 0, b""))

    def handle_leave(self):
        self.close_socket()
        self.render_main_menu()

    def close_socket(self):
        if self.socket is not None:
            self.socket.close()
            self.socket = None

    def start(self):
        self.root.update()

        def on_close():
            self.root.destroy()
            self.close_socket()
            sys.exit()

        self.root.protocol("WM_DELETE_WINDOW", on_close)

        self.render_main_menu()

        # Start the main loop
        self.root.mainloop()

def main():
    game = ScribbleGame()
    game.start()

if __name__ == "__main__":
    main()
