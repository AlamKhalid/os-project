from tkinter import ttk

import tkinter as tk

root = tk.Tk()
pid = 0

tree = ttk.Treeview(root, column=("c1", "c2", "c3", "c4", "c5"), show='headings')
tree.column("#1", anchor=tk.CENTER)
tree.heading("#1", text="PID")
tree.column("#2", anchor=tk.CENTER)
tree.heading("#2", text="USER")
tree.column("#3", anchor=tk.CENTER)
tree.heading("#3", text="CPU")
tree.column("#4", anchor=tk.CENTER)
tree.heading("#4", text="MEM")
tree.column("#5", anchor=tk.CENTER)
tree.heading("#5", text="CMD")
tree.pack()

# on click handler
def select_item(a):
    global pid
    item = tree.selection()[0]
    pid = tree.item(item)['values'][0]
# on click for button (end task)
def output():
    f = open('out.txt', 'w')
    f.write(str(pid))
    f.close()
# binding on click to table
tree.bind('<ButtonRelease-1>', select_item)

button1 = tk.Button(text="END TASK", command=output)
button1.pack(pady=10)

def refresh_table():
    tree.delete(*tree.get_children())
    # data from file
    procs = []
    f = open("proc.txt", "r")
    for x in f:
        line_list = x.split(' ')
        line_list[4] = line_list[4].replace("\n", "")
        procs.append(tuple(line_list))
    f.close()
    # insert in table
    for row in procs:
        tree.insert("", tk.END, values=row)
    # call again in 10 seconds
    root.after(2000, refresh_table)


root.title("Task Manager")
refresh_table()
root.mainloop()