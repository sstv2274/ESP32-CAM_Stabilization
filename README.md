# ESP32-CAM_Stabilization
This project focuses on real-time video stabilization and image processing using the ESP32-CAM platform, specifically optimized for FPV drone integration.
1. command : source /home/sinisa/venv/bin/activate
2. command : g++ -O3 -shared -o libstabilizacija.so -fPIC stabilizacija.cpp
3. command : QT_QPA_PLATFORM=xcb python prijemnik.py
