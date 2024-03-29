from tkinter import *


class Paint(object):
    def __init__(self, root: Tk):
        self.root = root

        # Color selectors
        self.colors = ["#FF55A8", "#FF452C", "#FFB61A", "#2EBF53",
                       "#288DD9", "#2133AB", "black", "white"]

        self.c = Canvas(self.root, bg='white', width=600, height=600)
        self.c.grid(row=1, columnspan=5)

        self.setup()

    def setup(self):
        self.old_x = None
        self.old_y = None

        self.c.place(x=50, y=150)

        for i, color in enumerate(self.colors):
            self.button = Button(self.root, bg=color, width=4, height=2,
                                 command=lambda color=color: self.handle_color_change(color))
            self.button.place(x=678, y=420 + 45 * i, anchor="center")
        
        self.button = Button(self.root, text="C", fg="white", bg="#2133AB", width=4, height=2, command=self.clear_canvas)
        self.button.place(x=678, y=150, anchor="n")

        self.active_color = "black"
        self.c.bind('<B1-Motion>', self.paint)
        self.c.bind('<ButtonRelease-1>', self.reset)

    def handle_color_change(self, color):
        print(color)
        self.active_color = color

    def paint(self, event):
        paint_color = self.active_color
        if self.old_x and self.old_y:
            self.c.create_line(self.old_x, self.old_y, event.x, event.y,
                               width=10, fill=paint_color,
                               capstyle=ROUND, smooth=TRUE, splinesteps=36)

        self.old_x = event.x
        self.old_y = event.y

    def clear_canvas(self):
        self.c.delete("all")

    def reset(self, event):
        self.old_x, self.old_y = None, None


if __name__ == '__main__':
    Paint()
