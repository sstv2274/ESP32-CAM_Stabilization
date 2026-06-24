#Sinisa Stevanovic RA58/2023
import numpy as np
import cv2
import ctypes
from os import path


lib_path = path.abspath("./libstabilizacija.so")
lib = ctypes.CDLL(lib_path)

# Definisanje argumenata za C++ funkciju
lib.stabilizuj_frejm.argtypes = [
    ctypes.POINTER(ctypes.c_ubyte), # data_ptr
    ctypes.c_int,                   # width
    ctypes.c_int,                   # height
    ctypes.c_int,                   # channels
    ctypes.c_int                    # stride (bajtova po redu)
]
lib.stabilizuj_frejm.restype = None 
ulazni_video = path.join("..", "video_snimci", "9.avi")
izlazni_video = path.join("..", "video_snimci", "9_stabilan.avi")

cap = cv2.VideoCapture(ulazni_video)

if not cap.isOpened():
    print(f"Greska: Ne mogu da otvorim fajl {ulazni_video}")
    exit()

# Preuzimamo karakteristike originalnog videa
w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fps = cap.get(cv2.CAP_PROP_FPS)
ukupno_frejmova = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))


izlazna_sirina = w
izlazna_visina = h // 2

# Inicijalizacija objekta za cuvanje videa
fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter(izlazni_video, fourcc, fps, (izlazna_sirina, izlazna_visina))

cv2.namedWindow('Obrada u toku', cv2.WINDOW_NORMAL)
print(f"Pocinjem obradu videa od {ukupno_frejmova} frejmova...")

frejm_brojac = 0


while True:
    ret, img = cap.read()
    
    if not ret:
        print("\nObrada zavrsena!")
        break
        
    frejm_brojac += 1
    
    if img is not None:
    
        original_img = img.copy()

        stride = img.strides[0] 
        data_ptr = img.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte))
        lib.stabilizuj_frejm(data_ptr, w, h, 3, stride)
        
        cv2.putText(original_img, "Original", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        cv2.putText(img, "Stabilizovano", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        
        kombinovani_prikaz = np.hstack((original_img, img))
        kombinovani_prikaz = cv2.resize(kombinovani_prikaz, (izlazna_sirina, izlazna_visina))
        
        out.write(kombinovani_prikaz)

        cv2.imshow('Obrada u toku', kombinovani_prikaz)
        
        print(f"\rObradjen frejm {frejm_brojac}/{ukupno_frejmova}", end="")

    if cv2.waitKey(1) == 27: 
        print("\nObrada prekinuta od strane korisnika.")
        break

cap.release()
out.release()
cv2.destroyAllWindows()
print(f"Video uspesno sacuvan kao: {izlazni_video}")