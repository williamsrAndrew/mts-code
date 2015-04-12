from tkinter import *

class App(object):
    def __init__(self, master):
        frame = Frame(master)
        frame.pack()

        TU = PhotoImage(file = "../tulogo.gif")
        TUimage = Label(frame, image = TU)
        TUimage.pack(side = RIGHT, padx = 3)

root = Tk()
app = App(root)
root.mainloop()