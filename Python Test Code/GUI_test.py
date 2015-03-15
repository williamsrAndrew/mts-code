## Motor Test GUI

from Tkinter import Tk, BOTH, RIGHT, RAISED, Listbox, END, LEFT
from ttk import Frame, Button, Style

class Motor(Frame):

	def __init__(self, parent):
		Frame.__init__(self, parent)
		self.parent = parent
		self.initUI()

	def initUI(self):
		self.parent.title("Motor Test")
		self.style = Style()
		# self.style.theme_use("clam")

		global lb
		lb = Listbox(self)
		lb.pack(fill = BOTH, expand = 1)

		frame = Frame(self, relief = RAISED, borderwidth = 1)
		frame.pack(fill = BOTH, expand = 1)

		self.pack(fill = BOTH, expand = 1)

		closeButton = Button(self, text = 'Close', command = self.quit)
		closeButton.pack(side = RIGHT, padx = 5, pady = 5)
		test1Button = Button(self, text = 'Test 1', command = self.runTest1)
		test1Button.pack(side = RIGHT)
		test2Button = Button(self, text = 'Test 2', command = self.runTest2)
		test2Button.pack(side = RIGHT)

		clearButton = Button(self, text = 'Clear', command = self.clearTxt)
		clearButton.pack(side = LEFT)

	def runTest1(self):
		lb.insert(END, 'Test 1 Complete')

	def runTest2(self):
		lb.insert(END, 'Test 2 Complete')

	def clearTxt(self):
		lb.delete(0, END)

def main():
	root = Tk()
	root.geometry("400x300")
	app = Motor(root)
	root.mainloop()

if __name__ == '__main__':
	main()


