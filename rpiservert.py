import socket
host="" 
port=8000

socket=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.bind((host, port))
socket.listen()
conn,addr=socket.accept()


data=socket.recv(1024)
dosya=open("/home/pi/Desktop/TURKSAT/time.txt","r+",encoding="utf-8")
dosya.write(data)