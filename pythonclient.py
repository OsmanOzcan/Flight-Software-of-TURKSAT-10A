import socket
import time
host = "192.168.137.124" #rp ip
port = 8888
import random

try:


# Socket oluşturulmasıx
    s = socket.socket()
# Bağlanılacak adres ve port
    

# Bağlantıyı yap
    s.connect((host, port))

    

    

   
    dosya = open("/home/pi/Desktop/TURKSAT/telemetry.txt", "r+", encoding="utf-8")
    #dosya=open("/home/pi/tlmtry.txt", "r+", encoding="utf-8")
    #rsp csv yolu
except socket.error as msg:
    print("Hata:", msg)
i = 0
while True:
    mesaj = dosya.readline()
    # mesaj="1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" + str(sayı)
    print(mesaj)

    s.send(mesaj.encode('utf-8'))


    #time.sleep(.5)

s.close()

