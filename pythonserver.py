import socket
import time
host = "" #rp ip
port = 8888
import random

try:

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("socket oluşturuldu")

    s.bind((host, port))
    print("socket {} nolu porta bağlandı".format(port))



# Client ile bağlantı kurulursa
    s.listen()
    print("socket dinleniyor")
    c, addr = s.accept()
    print(c)
    print('Gelen bağlantı:', addr)
    dosya = open("/home/pi/Desktop/TURKSAT/telemetry.txt", "r+", encoding="utf-8")
    #dosya=open("/home/pi/tlmtry.txt", "r+", encoding="utf-8")
    #rsp csv yolu
except socket.error as msg:
    print("Hata:", msg)
i = 0
while True:
    #sayı=random.randint(1,100)
    mesaj = dosya.readline()
    #mesaj="hiiiii" 
    print(mesaj)


    c.send(mesaj.encode('utf-8'))
    
    
    

    time.sleep(1)
# Bağlantıyı sonlandıralım
c.close()
