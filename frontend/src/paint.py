from tkinter import *
from typing import List
from packet_manager import *

class Paint(object):
    def __init__(self, root: Tk, on_draw: Callable[[CanvasPacket], None] = None, on_clear: Callable[[], None] = None):
        self.root = root
        self.on_draw = on_draw
        self.on_clear = on_clear

        # Color selectors
        self.colors = ["#FF55A8", "#FF452C", "#FFB61A", "#2EBF53",
                       "#288DD9", "#2133AB", "black", "white"]

        self.c = Canvas(self.root, bg='white', width=600, height=600)
        self.c.grid(row=1, columnspan=5)

        self.setup()

    def setup(self, is_drawer=True):
        self.old_x = None
        self.old_y = None
        self.is_drawer = is_drawer

        self.c.place(x=50, y=150)

        for i, color in enumerate(self.colors):
            self.button = Button(self.root, bg=color, width=4, height=2,
                                 command=lambda color=color: self.handle_color_change(color))
            self.button.place(x=678, y=420 + 45 * i, anchor="center")
        
        self.button = Button(self.root, text="C", fg="white", bg="#2133AB", width=4, height=2, command=self.handle_on_clear)
        self.button.place(x=678, y=150, anchor="n")

        self.active_color = "black"
        self.c.bind('<B1-Motion>', self.paint)
        self.c.bind('<ButtonRelease-1>', self.reset)

    def handle_color_change(self, color):
        print(color)
        self.active_color = color

    def paint(self, event):
        if self.is_drawer:
            paint_color = self.active_color

            if self.old_x and self.old_y:
                self.c.create_line(self.old_x, self.old_y, event.x, event.y,
                                width=10, fill=paint_color,
                                capstyle=ROUND, smooth=TRUE, splinesteps=36)
    
                self.send_drawing(self.old_x, self.old_y, event.x, event.y)
            else:
                self.send_drawing(event.x, event.y, event.x, event.y)
                
            self.old_x = event.x
            self.old_y = event.y    

    # Function to paint a canvas from the server
    def update_canvas(self, commands: List[CanvasPacket]):
        self.clear_canvas()

        # Draw each line from the server's list of commands
        for command in commands:
            self.c.create_line(command.old_x, command.old_y, command.new_x, command.new_y,
                               width=10, fill=self.colors[command.color],
                               capstyle=ROUND, smooth=TRUE, splinesteps=36)
            
    # Function to paint a new packet
    def paint_packet(self, packet: CanvasPacket):
        self.c.create_line(packet.old_x, packet.old_y, packet.new_x, packet.new_y,
                           width=10, fill=self.colors[packet.color],
                           capstyle=ROUND, smooth=TRUE, splinesteps=36)

    # Function to send canvas updates to server
    def send_drawing(self, o_x, o_y, n_x, n_y):
        packet = CanvasPacket(o_x, o_y, n_x, n_y, self.colors.index(self.active_color))
        if self.on_draw:
            self.on_draw(packet)

    def handle_on_clear(self):
        if self.is_drawer:
            self.clear_canvas()
        if self.is_drawer and self.on_clear:
            self.on_clear()

    def clear_canvas(self):
        self.c.delete("all")
        self.reset(None)

    def reset(self, event):
        self.old_x, self.old_y = None, None


if __name__ == '__main__':
    Paint()
