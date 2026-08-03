import random
from math import *

print("hello world", end=" ")
print("hello world")
#这是一行注释
"""
这是多行注释
"""
name = "xxx"
age = 19
mylist = ["路飞", "伊姆"]
print("xxx %s%d"%(name,age))

xsxs = 3.99999999
print("%.2f"%xsxs)#四舍五入保留两位小数
print(f"my name is {name},my age is{age}")

#while age != 10:
#    print("hello world")
#    age -= 1

#range(num1, num2, step) 重新构建了一个数字序列
"""
#九九乘法表
for i in range(1, 10):#临时变量i从1遍历到9
    for a in range(1, i+1):#从1遍历到i
        print("{num1} * {num2} = {num3}".format(num1=i, num2=i+1, num3= i * (i+1)), end=" ")
    print()

"""

"""
#猜数字比大小
num = random.randint(1, 100)
print(num)
guessnum = num - 1
while guessnum != num:
    try:
        guessnum = int(input("输入你想要猜的数字："))
    except ValueError:
        print("只能用数字")
    match guessnum:
        case a if a < num:
            print("你猜小了")
        case a if a > num:
            print("你猜大了")

else:
    print("你猜对了")
"""

#列表可以修改，元组一旦定义不可以修改
print(mylist)
mylist[0] = "乔伊波伊"
print(mylist)
print(mylist.index("伊姆"))#这个元素对应的下标
print(mylist)
mylist.insert(2, "戴维琼斯")
mylist.insert(1, "黑胡子")#指定位置插入
print(mylist.index("戴维琼斯"))
print(mylist)
mylist.append("one piece")#加在末尾
mylist2 = ["鸣人", "佐助"]
mylist.extend(mylist2)
print(mylist.pop(3))#删除指定下标元素并且返回这个值
"""
del mylist[下标]

mylist.remove(xxx)#从前往后检索，只删除一遍

mylist.clear()#清空列表

count(xxx)#xxx在列表中的数量
len(mylist)#一共多少个元素
"""
print(mylist)

class pony:
    #init函数是在创建实例时候才会执行，所以这时候可以调用set_ponyage这个函数
    def __init__(self, pony_age, pony_list = None):
        self.set_ponyage(pony_age)
        self.ponylist = pony_list
        self.class_list = []
    def __repr__(self):
        return "this pony's age is {num1},list is {num2}".format(num1 = self._ponyage, num2 = self.ponylist)
    """
    python中的重要概念@property
    用于定义类中的私有变量，这样就可以在新的实例中完成相应操作，
    而不是像全局变量一样
    """
    def set_ponyage(self, nweage):
        self._ponyage = nweage
    def get_ponyage(self):
        #raise ValueError("李成哲是个傻逼")
        return(self._ponyage)

new1_pony = pony(1, None)
new2_pony = pony(2, None)

new1_pony.class_list.append("微风")

print(new1_pony.class_list)
print(new2_pony.class_list)
print(new1_pony)
new1_pony.get_ponyage()

#continue实例，用于取奇数
for i in range(0,11):
    if i % 2 == 0:
        continue
    print(i, end=" ")

print()

def Ponyfunction(func):
    #定义一个打包器，并且接受传入变量
    def wrapper(*args, **kwargs):
        #这里就是将原来的my_function存起来
        result = func(*args, **kwargs)
        if func() == 3:
            print("装饰成功")
        else:
            print("失败")
        #这里就是可以再次调用原函数
        return result
    return wrapper
#在 Python 中，装饰器（@Ponyfunction）是在“定义函数”的那一刻执行的，
# 而不是在“调用函数”的时候执行的。
@Ponyfunction
def my_function():
    return 3
"""
这里的运行逻辑是先定义好Ponyfunction然后装饰my_function
(就是将my function打包给ponyfunction)
然后利用内置函数完成两个函数的融合
"""

print(my_function())


import tkinter as tk
import time
index = 0
random_text = ["爱你哟"]
root = tk.Tk()
root.withdraw()
def lele():
    pass
def create(text_num=0):
    global index
    if index >= 50:
        return
    t = 2 * pi * (index/50)
    love_x = (16 * (sin(t) ** 3))*25
    love_y = -(13 * cos(t) - 5 * cos(2*t) - 2 * cos(3*t) - cos(4*t))*25
    window = tk.Toplevel(root)
    window.title("爱你喵")

    width = 225
    height = 100
    window.geometry(f"{width}x{height}+{int(love_x+741)}+{int(love_y+483)}")
    text = tk.Label(window, text=f"{random_text[text_num]}")
    text.grid(row=0, column=0, sticky="nsew")
    window.grid_rowconfigure(0, weight=1)
    window.grid_columnconfigure(0, weight=1)
    #window.after(1000, create, x, y)

    window.after(50,create)
    index += 1
# 主窗口
root = tk.Tk()
root.withdraw()
create()
root.mainloop()

print(1)
print(1)