"""Maintains the screen of the program and its children"""

import tkinter as tk


class ScreenManager:
    __instance: None

    @staticmethod
    def get_instance() -> "ScreenManager":
        if ScreenManager.__instance is None:
            ScreenManager().__instance = ScreenManager()
        return ScreenManager.__instance

    def __init__(self):
        ScreenManager.__instance = self
        self.__root = tk.Tk()
        self.__root.title("Pixelnary")
        self.__root.configure(bg="#2133AB")
        self.__root.geometry("1200x800")
        # self.__root.iconbitmap() TODO: Add icon
        self.__root.resizable(False, False)

        self.__heading_font = ("Arial", 72)
        self.__subheading_font = ("Arial", 32)
        self.__text_font = ("Arial", 12)

    def clear_screen(self):
        for widget in self.__root.winfo_children():
            widget.destroy()

    def get_root(self):
        return self.__root

    def get_heading_font(self):
        return self.__heading_font

    def get_subheading_font(self):
        return self.__subheading_font

    def get_text_font(self):
        return self.__text_font


# Create a module-level instance of ScreenManager
screen_manager = ScreenManager()
