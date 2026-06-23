#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib> 
#include <algorithm>

struct KeyPoint {
    int x;
    int y;
    float angle;
    int score; //Quality of point
    unsigned char descriptor[32]; 
};

//Offsets for circle
const int krug_x[16] = { 0,  1,  2,  3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1 };
const int krug_y[16] = {-3, -3, -2, -1, 0, 1, 2, 3, 3,  3,  2,  1,  0, -1, -2, -3 };

//Global matrix for BRIEF
static int par_Ax[256], par_Ay[256];
static int par_Bx[256], par_By[256];
static bool parovi_generisani = false;

void generisi_brief_parove() {
    std::srand(42); 
    for(int i = 0; i < 256; i++) {
        par_Ax[i] = std::rand() % 31 - 15; 
        par_Ay[i] = std::rand() % 31 - 15;
        par_Bx[i] = std::rand() % 31 - 15;
        par_By[i] = std::rand() % 31 - 15;
    }
    parovi_generisani = true;
}

//Fast-9 edge detection with(returning score)
int jel_fast_tacka_sa_skorom(const std::vector<unsigned char>& gray, int cx, int cy, int width, int height, int threshold) {
    int p_idx = cy * width + cx;
    unsigned char p_val = gray[p_idx];
    int provera_idx[4] = {0, 4, 8, 12};
    int svetliji_brojac = 0, tamniji_brojac = 0;

    for (int i = 0; i < 4; i++) {
        int k = provera_idx[i];
        unsigned char n_val = gray[(cy + krug_y[k]) * width + (cx + krug_x[k])];
        if (n_val > p_val + threshold) svetliji_brojac++;
        if (n_val < p_val - threshold) tamniji_brojac++;
    }
    if (svetliji_brojac < 3 && tamniji_brojac < 3) return 0;

    unsigned char v[16];
    for (int i = 0; i < 16; i++) {
        v[i] = gray[(cy + krug_y[i]) * width + (cx + krug_x[i])];
    }

    bool jeste_tacka = false;
    for (int i = 0; i < 16; i++) {
        bool niz_svetliji = true, niz_tamniji = true;
        for (int n = 0; n < 9; n++) {
            int idx = (i + n) % 16;
            if (v[idx] <= p_val + threshold) niz_svetliji = false;
            if (v[idx] >= p_val - threshold) niz_tamniji = false;
        }
        if (niz_svetliji || niz_tamniji) {
            jeste_tacka = true;
            break;
        }
    }

    if (!jeste_tacka) return 0;

    //Calculating score
    int skor = 0;
    for (int i = 0; i < 16; i++) {
        skor += std::abs(v[i] - p_val);
    }
    return skor;
}

// orientation
float izracunaj_orijentaciju(const std::vector<unsigned char>& gray, int cx, int cy, int width, int height) {
    long long m10 = 0, m01 = 0;
    int poluprecnik = 7;
    for (int y = -poluprecnik; y <= poluprecnik; y++) {
        for (int x = -poluprecnik; x <= poluprecnik; x++) {
            int nx = cx + x, ny = cy + y;
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                unsigned char intenzitet = gray[ny * width + nx];
                m10 += x * intenzitet;
                m01 += y * intenzitet;
            }
        }
    }
    return std::atan2(m01, m10);
}

int izracunaj_hamingovu(const unsigned char* d1, const unsigned char* d2) {
    int distanca = 0;
    for (int i = 0; i < 32; i++) {
        distanca += __builtin_popcount(d1[i] ^ d2[i]);
    }
    return distanca;
}

//Sorting
bool sortiraj_po_skoru(const KeyPoint& a, const KeyPoint& b) {
    return a.score > b.score;
}

extern "C" {
    void stabilizuj_frejm(unsigned char *data, int width, int height, int channels, int stride) {
        if (!parovi_generisani) {
            generisi_brief_parove();
        }

        std::vector<unsigned char> gray(width * height);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * stride) + (x * channels);
                unsigned char b = data[index];
                unsigned char g = data[index + 1];
                unsigned char r = data[index + 2];
                
                unsigned char y_val = (unsigned char)((114 * b + 587 * g + 299 * r) / 1000);
                gray[y * width + x] = y_val;

                //data[index]     = y_val;
                //data[index + 1] = y_val;
                //data[index + 2] = y_val;
            }
        }

        std::vector<KeyPoint> sve_tacke;
        int prag = 8; //Small because of moting blur

        //taking all points and their scores
        for (int y = 15; y < height - 15; y++) {
            for (int x = 15; x < width - 15; x++) {
                int skor = jel_fast_tacka_sa_skorom(gray, x, y, width, height, prag);
                if (skor > 0) {
                    KeyPoint kp;
                    kp.x = x;
                    kp.y = y;
                    kp.score = skor;
                    sve_tacke.push_back(kp);
                }
            }
        }

        //Filter for max number of points
        std::sort(sve_tacke.begin(), sve_tacke.end(), sortiraj_po_skoru);

        //max 150 points
        int max_tacaka = 150;
        if (sve_tacke.size() > max_tacaka) {
            sve_tacke.resize(max_tacaka);
        }

        //calculating Brief descriptors
        std::vector<KeyPoint> kljucne_tacke;
        for (auto& kp : sve_tacke) {
            kp.angle = izracunaj_orijentaciju(gray, kp.x, kp.y, width, height);
            
            for(int d = 0; d < 32; d++) kp.descriptor[d] = 0;

            float cos_t = std::cos(kp.angle);
            float sin_t = std::sin(kp.angle);

            for(int i = 0; i < 256; i++) {
                int ax_rot = std::round(par_Ax[i] * cos_t - par_Ay[i] * sin_t);
                int ay_rot = std::round(par_Ax[i] * sin_t + par_Ay[i] * cos_t);
                int bx_rot = std::round(par_Bx[i] * cos_t - par_By[i] * sin_t);
                int by_rot = std::round(par_Bx[i] * sin_t + par_By[i] * cos_t);
                
                int pA = gray[(kp.y + ay_rot) * width + (kp.x + ax_rot)];
                int pB = gray[(kp.y + by_rot) * width + (kp.x + bx_rot)];
                
                if (pA < pB) {
                    kp.descriptor[i / 8] |= (1 << (i % 8));
                }
            }
            kljucne_tacke.push_back(kp);
        }

        
        static std::vector<KeyPoint> prethodne_tacke;
        
        if (!prethodne_tacke.empty() && !kljucne_tacke.empty()) {
            for (const auto& curr : kljucne_tacke) {
                int najbolja_distanca = 256;

                for (const auto& prev : prethodne_tacke) {
                    int dist = izracunaj_hamingovu(curr.descriptor, prev.descriptor);
                    if (dist < najbolja_distanca) {
                        najbolja_distanca = dist;
                    }
                }

                if (najbolja_distanca < 45) {
                    //coloring picture
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            int px = curr.x + dx;
                            int py = curr.y + dy;
                            if (px >= 0 && px < width && py >= 0 && py < height) {
                                int bgr_idx = (py * stride) + (px * channels);
                                data[bgr_idx]     = 255; 
                                data[bgr_idx + 1] = 0;   
                                data[bgr_idx + 2] = 0;   
                            }
                        }
                    }
                }
            }
        }

        prethodne_tacke = kljucne_tacke;
    }
}