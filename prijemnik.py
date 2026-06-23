import socket
import numpy as np
import cv2

# Test: Kreiraj prozor odmah
cv2.namedWindow('ESP32-CAM', cv2.WINDOW_AUTOSIZE)
print("Prozor otvoren. Čekam podatke...")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# ... ostatak koda ...
# Povećaj bafer mrežne kartice da ne gubiš pakete
sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)
sock.bind(("0.0.0.0", 8888))

print("Slušam...")

data = bytearray()

while True:
    packet, addr = sock.recvfrom(4096)
    data.extend(packet)
    
    # DEBUG: Da li uopšte vidimo JPEG početak?
    if len(data) > 2 and data[0:2] == b'\xff\xd8':
        # Samo proveri da li imamo kraja
        if b'\xff\xd9' in data:
            print("Frejm primljen! Dužina:", len(data))
            # Nastavi sa dekodiranjem...
            nparr = np.frombuffer(data, np.uint8)
            img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            if img is not None:
                print(f"Dekodirana dimenzija: {img.shape}") # OVO JE KLJUČNO!
                cv2.imshow('ESP32-CAM', img)
                cv2.waitKey(1) # Ovo mora biti tu da bi se prozor iscrtao
            data = bytearray()
    else:
        # Ako ne počinje sa FF D8, to je smeće (nepotpun paket), bacamo ga
        if len(data) > 0:
            data = bytearray()

cv2.destroyAllWindows()