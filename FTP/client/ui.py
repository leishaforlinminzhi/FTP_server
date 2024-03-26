import socket
import math
import sys
import tkinter as tk
from tkinter import ttk
import tkinter.messagebox as messageb
import ttkbootstrap as ttkb
from ttkbootstrap.constants import *

import client

class Index(tk.Frame):
    def __init__(self,parent=None):
        super().__init__(parent)
        self.pack(expand=1,fill="both")
        self.frame_left = tk.Frame(self)
        self.frame_left.pack(side='left',expand=1,fill="x",padx=5,pady=5)
        # defferent pages
        for i in ["连接服务器","发送请求","历史记录"]:
            but = ttkb.Button(self.frame_left,text=i,bootstyle=(ttkb.INFO, ttkb.OUTLINE))
            but.pack(side='top',expand=1,fill="y",padx=5, pady=10)
            but.bind("<Button-1>",self.change) 
        # page content     
        self.frame_right=tk.Frame(self)
        self.frame_right.pack(side='left',expand=1,fill="both",padx=5,pady=5)
        lab = tk.Label(self.frame_right,text="欢迎使用FTP client!\n by lmz",width=30)
        lab.pack()

        #根据鼠标左键单击事件，切换页面
    def change(self,event):
        res = event.widget["text"]
        for i in self.frame_right.winfo_children():
            i.destroy()
        if res == "连接服务器":
            Page1(self.frame_right)
        elif res == "发送请求":
            Page2(self.frame_right)
        elif res == "历史记录":
            Page3(self.frame_right)

            
class Page1(tk.Frame):
    def __init__(self,parent=None):
        super().__init__(parent)
        self.pack(expand=1,fill="both")

        Page1.label = tk.Label(self,text="请输入服务器IP和端口号")
        Page1.label.pack()

        ip_frame = tk.Frame(self)
        ip_frame.pack()
        label_ip = tk.Label(ip_frame, text="ip in format \"%d.%d.%d.%d\":")
        label_ip.pack()
        Page1.entry_ip = tk.Entry(ip_frame, width=30) # 将entry_ip变为Page1的一个类属性
        Page1.entry_ip.pack()

        port_frame = tk.Frame(self)
        port_frame.pack()
        label_port = tk.Label(port_frame, text="port:")
        label_port.pack()
        Page1.entry_port = tk.Entry(port_frame, width=30) # 将entry_ip变为Page1的一个类属性
        Page1.entry_port.pack()

        buttun_connect = ttkb.Button(self,text="连接",bootstyle=(ttkb.INFO, ttkb.OUTLINE),command=client.init_server_connect)
        buttun_connect.pack()
        
    
class Page2(tk.Frame):
    def __init__(self,parent=None):
        super().__init__(parent)
        self.pack(expand=1,fill="both")

        Page2.label = tk.Label(self,text="请选择向服务器发送的请求")
        Page2.label.pack()

        Page2.selected_option = tk.StringVar()  # 用于存储选择的选项
        Page2.dropdown = ttk.Combobox(self, textvariable=Page2.selected_option, width=30)
        Page2.dropdown['values'] = ('USER', 'PASS', 'RETR', 'STOR', 'QUIT', 'SYST','TYPE', 
                                    'PORT', 'PASV', 'MKD', 'CWD', 'PWD', 'LIST', 'RMD', 
                                    'RNFR', 'RNTO')  # 可供选择的选项
        Page2.dropdown.pack()

        buttun_determine = ttkb.Button(self,text="确定",bootstyle=(ttkb.INFO, ttkb.OUTLINE),command=client.handle_request)
        buttun_determine.pack()

        Page2.label_para = tk.Label(self,text="请输入请求对应参数")
        Page2.entry_command = tk.Entry(self, width=30)
        Page2.buttun_send = ttkb.Button(self,text="发送",bootstyle=(ttkb.INFO, ttkb.OUTLINE),command=client.send_request)
        # Page2.entry_command.pack()

        Page2.text_recv = tk.Text(self,width=40,height=30,state="disabled")
        # Page2.text_recv.pack()
        

        
class Page3(tk.Frame):
    def __init__(self,parent=None):
        super().__init__(parent)
        self.pack(expand=1,fill="both")

        Page3.text_history = tk.Text(self,width=40,height=30)
        Page3.text_history.pack()
        Page3.text_history.insert(1.0,client.info.buffer_history)
        Page3.text_history.configure(state="disabled")