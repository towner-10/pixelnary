"""Handles main frontend functionality"""

import tkinter as tk
import screen_manager as sm
import sys


def create_main_menu(root: tk.Tk, sm: sm.ScreenManager):
    heading = sm.get_heading_font()
    subheading = sm.get_subheading_font()
    text = sm.get_text_font()
    text_bold = (text[0], text[1], "bold")
    # center_x = sm.get_center_x() TODO: Fix

    # Title
    title_label = tk.Label(root, text="Pixelnary", font=heading, bg="#2133AB", fg="white")
    title_label.place(x=600, y=260, anchor="center")

    # Subtitle
    subtitle_label = tk.Label(root, text="SE 3313B Stretch Project", font=text_bold, bg="#2133AB", fg="white")
    subtitle_label.place(x=600, y=325, anchor="center")

    # Join button
    join_button = tk.Button(root, text="Join an existing room", font=text_bold, bg="#2EBF53", fg="white", width=40, height=3)
    join_button.place(x=600, y=500, anchor="center")

    # Create button
    create_button = tk.Button(root, text="Create a new room", font=text_bold, bg="#288DD9", fg="white", width=40, height=2)
    create_button.place(x=600, y=575, anchor="center")

def main():
    # Set the screen manager and root
    screen_manager: sm.ScreenManager = sm.screen_manager.get_instance()
    root = screen_manager.get_root()
    root.update()

    def on_close():
        root.destroy()
        sys.exit()

    root.protocol("WM_DELETE_WINDOW", on_close)

    create_main_menu(root, screen_manager)

    # Start the main loop
    root.mainloop()


if __name__ == "__main__":
    main()
