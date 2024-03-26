import socket
import math
import sys
import tkinter as tk
from tkinter import filedialog
import tkinter.messagebox as messageb
import ttkbootstrap as ttkb
import re

import ui

class info:
    init_flag = 0
    client_IP = "127.0.0.1"
    client_port = 1
    server_IP = "127.0.0.1"
    server_port = 1
    data_IP = "127.0.0.1"
    data_port = 20001

    binary_flag = 0

    server_socket = socket.socket()
    data_socket = socket.socket()

    connected_flag = "" # PORT or PASV

    buffer_history = ""

    connect_to_server_flag = 0

    request = ""
    selected = ""

    conn = 0

def init_server_connect():
    entry_ip = ui.Page1.entry_ip # 获取Page1类中的entry_ip对象
    entry_port = ui.Page1.entry_port
    print(entry_ip.get(),entry_port.get())

    try:
        info.server_IP = entry_ip.get()  # server IP
        info.server_port = int(entry_port.get())  # server port
        info.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        info.server_socket.connect((info.server_IP, info.server_port))
    except:
        ui.Page1.label.config(text="无法连接到服务器：" + entry_ip.get() + ",端口号：" + entry_port.get()+",请重试。")

    response = info.server_socket.recv(8192).decode()
    print(response)

    ui.Page1.label.config(text="成功连接到服务器：" + entry_ip.get() + ",端口号：" + entry_port.get())
    print("successfully init!\n")
    info.connect_to_server_flag = 1
    return True

def clear_data_connect():
    info.data_socket.close()
    info.data_socket = None
    info.data_socket = socket.socket()

def connect_to_pasv():
    try:
        info.data_socket.connect((info.server_IP, info.data_port))
    except socket.error as e:
        print(e)
        return False
    print(info.data_port)
    return True

def handle_port(msg):
    parameter = msg[1].split(',')
    if not (len(parameter)==6):
        return False
    info.data_IP = parameter[0]+"."+parameter[1]+"."+parameter[2]+"."+parameter[3]
    info.data_port = int(parameter[4])*256+int(parameter[5])
    print(info.data_IP,info.data_port)

    clear_data_connect()

    try:
        info.data_socket=socket.socket()
        info.data_socket.bind((info.client_IP,info.data_port))
        info.data_socket.listen(5)
    except:
        return False
    info.connected_flag = "PORT"
    return True

def handle_pasv(recv_msg):
    pattern = r"\d+,\d+,\d+,\d+,\d+,\d+"  # 正则表达式模式
    match = re.search(pattern, recv_msg)  # 在字符串中查找匹配项
    if not match:
        return False
    numbers_str = match.group(0)  # 提取匹配到的字符串
    parameter = [int(num) for num in numbers_str.split(",")]  # 将字符串转换为数字列表
    info.data_IP = str(parameter[0])+"."+str(parameter[1])+"."+str(parameter[2])+"."+str(parameter[3])
    info.data_port = int(parameter[4])*256+int(parameter[5])
    print(info.data_IP,info.data_port)

    clear_data_connect()
    info.connected_flag = "PASV"
    return True

def handle_retr(msg):
    if(info.connected_flag == "PORT"):
        info.conn,addr = info.data_socket.accept()
    file_name=msg[1].split('/')[-1]
    try:
        file = open(file_name, "wb+")
    except Exception as e:
        print(e)  
        return False
    
    while(True):
        if(info.connected_flag == "PASV"):
            buffer = info.data_socket.recv(8192)
        else:
            print(1)
            buffer = info.conn.recv(8192)
        if buffer == None or len(buffer) == 0 or (not buffer):
            break
        try:
            file.write(buffer)
        except socket.error as e:
            print(e)
            return False
        
    ui.Page2.text_recv.insert(1.0,buffer)
    try:
        file.close()
    except:
        return False
    
    if(info.connected_flag == "PORT"):
        info.conn.close()
    else:
        clear_data_connect()
    return True

def handle_stor(msg):
    if(info.connected_flag == "PORT"):
        info.conn,addr = info.data_socket.accept()
    try:
        file = open(msg[1].split("/")[-1], "rb")
    except:
        return "file does not exist"
    while(True):
        line = file.read(8192)
        if(info.connected_flag == "PASV"):
            info.data_socket.send(line)
        else:
            info.conn.send(line)
        if not line:
            break
    if(info.connected_flag == "PASV"):
        info.data_socket.send(b'\0')
    else:
        info.conn.send(b'\0')

    if(info.connected_flag == "PORT"):
        info.conn.close()
    else:
        clear_data_connect()
    return "ok"

def handle_list():
    while(True):
        if(info.connected_flag == "PASV"):
            buffer = info.data_socket.recv(8192)
        else:
            buffer = info.conn.recv(8192)
        if buffer == None or len(buffer) == 0 or (not buffer):
            break
        print(buffer)
        ui.Page2.text_recv.insert(1.0,buffer)
    if(info.connected_flag == "PORT"):
        info.conn.close()
    else:
        clear_data_connect()


def handle_type(msg):
    if("I" in msg):
        info.binary_flag = 1
        print("type to 1")
    elif("A" in msg):
        info.binary_flag = 0
        print("type to 0")
    else:
        print("type ?\n")
    

def handle_request():
    ui.Page2.label_para.pack_forget()
    ui.Page2.entry_command.pack_forget()
    ui.Page2.buttun_send.pack_forget()

    if not (info.connect_to_server_flag):
        ui.Page2.label.config(text="请先连接到服务器，再发送请求。")
        return False
    
    info.selected = ui.Page2.selected_option.get()
    print("选择的选项:", info.selected)

    dict_prompt = {'USER':'请输入您的用户名',
                   'PASS':'请输入您的密码',
                   'RETR':'请输入您要下载的文件路径，当前路径下只需名称',
                   'TYPE':'请输入您要更改的状态',
                   'PORT':'请输入您要指定的IP和端口号',
                   'MKD':'请输入您新建文件夹的文件路径，当前路径下只需名称',
                   'CWD':'请输入您要前往的绝对路径或相对路径',
                   'RMD':'请输入您要删除的文件夹的文件路径，当前路径下只需名称',
                   'RNFR':'请输入您要重命名的文件路径，当前路径下只需名称',
                   'RNTO':'请输入您要重命名的新文件名称'}
    
    if (info.selected == 'STOR'):
        file_path = filedialog.askopenfilename()
        print(file_path)
        info.request = info.selected + " " +file_path.split("/")[-1]
        send_request()
    elif (info.selected == 'PASV' or info.selected == 'SYST' or info.selected == 'QUIT' 
            or info.selected == 'PWD' or info.selected == 'LIST'):
        info.request = info.selected
        send_request()
    else:
        ui.Page2.text_recv.pack_forget()
        ui.Page2.label_para.config(text=dict_prompt[info.selected])
        ui.Page2.label_para.pack()
        ui.Page2.entry_command.pack()
        ui.Page2.buttun_send.pack()
        print("show")
    
    return True



def send_request():
    entry_command = ui.Page2.entry_command
    command = entry_command.get()
    if not (len(command) or info.selected == 'PASV' or info.selected == 'SYST' or info.selected == 'QUIT' 
            or info.selected == 'PWD' or info.selected == 'LIST' or info.selected == 'STOR'):
        return False
    if not (info.selected == 'PASV' or info.selected == 'SYST' or info.selected == 'QUIT' 
        or info.selected == 'STOR' or info.selected == 'PWD' or info.selected == 'LIST'):
        info.request = info.selected + ' ' + command

    ui.Page2.entry_command.delete(0,tk.END)
    ui.Page2.dropdown.delete(0,tk.END)
    ui.Page2.label_para.pack_forget()
    ui.Page2.entry_command.pack_forget()
    ui.Page2.buttun_send.pack_forget()
    

    command = info.request + '\r\n'
    print("{"+command+"}")
    if not (len(command)):
        return False
    
    msg = info.request.split(" ")

    if(msg[0] =='STOR' or msg[0] =='RETR' or msg[0] =='LIST'):
        if(info.connected_flag == "PORT"):
            print(1)
        elif(info.connected_flag == "PASV"):
            if not connect_to_pasv():
                print("fail to connect to pasv\n")
                ui.Page2.text_recv.insert(1.0,"fail to connect to pasv")
                return False
    
    info.server_socket.send(str.encode(command))
    buffer = info.server_socket.recv(8192).decode()
    info.buffer_history = "client:" + info.request + "\nserver:" + buffer + info.buffer_history

    ui.Page2.text_recv.configure(state="normal")
    ui.Page2.text_recv.insert(1.0,"client:" + info.request + "\nserver:" + buffer)

    recv_msg = buffer.split(" ")

    print(recv_msg)
    print(msg)
    if (msg[0] == "RETR" and recv_msg[0] == "150"):
        print (handle_retr(msg))
        buffer = info.server_socket.recv(8192).decode('UTF-8','strict')
        info.buffer_history = "server:" + buffer + info.buffer_history
        ui.Page2.text_recv.insert(1.0,buffer)
    if (msg[0] == "STOR" and recv_msg[0] == "150"):
        print(handle_stor(msg))
        buffer = info.server_socket.recv(8192).decode('UTF-8','strict')
        info.buffer_history = "server:" + buffer + info.buffer_history
        ui.Page2.text_recv.insert(1.0,buffer)
    if (msg[0] == "PORT" and recv_msg[0] == "200"):
        handle_port(msg)
    if (msg[0] == "PASV" and recv_msg[0] == "227"):
        handle_pasv(buffer)
    if (msg[0] == "TYPE" and recv_msg[0] == "200"):
        handle_type(buffer)
    if (msg[0] == "LIST"):
        handle_list()
        buffer = info.server_socket.recv(8192).decode('UTF-8','strict')
        info.buffer_history = "server:" + buffer + info.buffer_history
        ui.Page2.text_recv.insert(1.0,buffer)

    ui.Page2.text_recv.configure(state="disabled")
    ui.Page2.text_recv.pack()

    return True

if __name__ == "__main__":
    style = ttkb.Style()
    style = ttkb.Style(theme='minty')
    #想要切换主题，修改theme值即可['vista', 'cyborg', 'journal', 'darkly', 'flatly', 'solar', 'minty', 'litera', 'united' 'pulse', 'cosmo', 'lumen', 'yeti', 'superhero', 'winnative', 'sandstone', 'default']
    root = style.master
    # root = tk.Tk()
    root.title("FTP client")
    root.geometry("600x400") 
    index = ui.Index(root)
    root.mainloop()

