import socket
import time

# Socket oluşturulmasıx
s = socket.socket()

# Bağlanılacak adres ve port
host = "192.168.137.124" #yer ist ip
port = 2300
# Bağlantıyı yap
s.connect((host, port))

while True:
    # serverden yanıtı al
    yanit = s.recv(18000)
    telemetri = yanit.decode("utf-8")

    

    print(yanit.decode("utf-8"))
    with open('/home/pi/Desktop/TURKSAT/GCS.txt', "w", encoding="utf-8") as dosya:

        dosya.write(telemetri)

# bağlantıyı kapat
s.close()
