"""Handles main frontend functionality"""

import tkinter as tk
import screen_manager as sm
# from pydub import AudioSegment
# from pydub.playback import play
import sys


def render_main_menu(root: tk.Tk, sm: sm.ScreenManager):
    sm.clear_screen()

    heading = sm.get_heading_font()
    text = sm.get_text_font()
    text_bold = (text[0], text[1], "bold")

    # Game title
    title_label = tk.Label(root, text="Pixelnary",
                           font=heading, bg="#2133AB", fg="white")
    title_label.place(x=600, y=260, anchor="center")

    # Subtitle
    subtitle_label = tk.Label(
        root, text="SE 3313B Stretch Project", font=text_bold, bg="#2133AB", fg="white")
    subtitle_label.place(x=600, y=325, anchor="center")

    # Room ID join input
    input_entry = tk.Entry(root, font=text, width=16)
    input_entry.place(x=470, y=500)

    # Join button
    join_button = tk.Button(root, text="Join room",
                            font=text_bold, bg="#2EBF53", fg="white", width=10, height=1, command=lambda: handle_join(root, sm))
    join_button.place(x=675, y=510, anchor="center")

    # Create button
    create_button = tk.Button(root, text="Create a new room",
                              font=text_bold, bg="#288DD9", fg="white", width=25, height=2, command=lambda: handle_create(root, sm))
    create_button.place(x=600, y=575, anchor="center")


def handle_join(root: tk.Tk, sm: sm.ScreenManager):
    # TODO: Backend integration

    # Play join sound TODO: Fix
    # join_sound = AudioSegment.from_wav("frontend/sounds/join.wav")
    # play(join_sound)

    # TODO: Replace placeholder room ID and drawer + add word
    render_room(root, sm, "123456", False)


def handle_create(root: tk.Tk, sm: sm.ScreenManager):
    # TODO: Backend integration

    # TODO: Replace placeholder room ID and drawer + add word
    render_room(root, sm, "123456", False)


def handle_leave(root: tk.Tk, sm: sm.ScreenManager):
    # TODO: Backend integration

    render_main_menu(root, sm)


def render_room(root: tk.Tk, sm: sm.ScreenManager, room_id: str, drawer: bool, word: str = None):
    sm.clear_screen()

    subheading = sm.get_subheading_font()
    subheading_bold = (subheading[0], subheading[1], "bold")
    text = sm.get_text_font()
    text_bold = (text[0], text[1], "bold")

    # Leave button
    leave_button = tk.Button(root, text="Leave",
                             font=text_bold, bg="#288DD9", fg="white", width=6, height=1, command=lambda: handle_leave(root, sm))
    leave_button.place(x=50, y=62)

    # Room ID title
    room_label = tk.Label(root, text="Room",
                          font=subheading_bold, bg="#2133AB", fg="white")
    room_label.place(x=165, y=50)
    id_label = tk.Label(root, text=room_id,
                        font=subheading, bg="#2133AB", fg="white")
    id_label.place(x=295, y=50)

    # Canvas
    canvas = tk.Canvas(root, width=600, height=600, bg="white")
    canvas.place(x=50, y=150)

    # Color selectors
    colors = ["#FF55A8", "#FF452C", "#FFB61A", "#2EBF53",
              "#288DD9", "#2133AB", "black", "white"]

    for i, color in enumerate(colors):
        button = tk.Button(root, bg=color, width=4, height=2)
        button.place(x=678, y=420 + 45 * i, anchor="center")

    # Action label
    if (drawer):
        action = f"Your word is {word}. Draw it!"
    else:
        action = "Guess the word!"

    action_label = tk.Label(root, text=action,
                            font=text_bold, bg="#2133AB", fg="white")
    action_label.place(x=950, y=160, anchor="center")

    # Chat box
    box = tk.Canvas(root, width=400, height=550, bg="#16208F")
    box.place(x=750, y=200)

    # Message input
    msg_entry = tk.Entry(root, font=text, width=34)
    msg_entry.place(x=762, y=720)

    # Guess button
    guess_button = tk.Button(root, text="Guess",
                             font=text_bold, bg="#288DD9", fg="white", width=6, height=1, command=lambda: handle_join(root, sm))
    guess_button.place(x=1109, y=729, anchor="center")


def main():
    # Set the screen manager and root
    screen_manager: sm.ScreenManager = sm.screen_manager.get_instance()
    root = screen_manager.get_root()
    root.update()

    def on_close():
        root.destroy()
        sys.exit()

    root.protocol("WM_DELETE_WINDOW", on_close)

    render_main_menu(root, screen_manager)

    # Start the main loop
    root.mainloop()


if __name__ == "__main__":
    main()
