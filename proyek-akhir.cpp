#include <glut.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>

// Konstanta global
const int LEBAR_JENDELA = 800;
const int TINGGI_JENDELA = 600;
const int JUMLAH_BARIS = 15;
const int JUMLAH_KOLOM = 15;
const float UKURAN_GRID = 1.0f;
const float UKURAN_KUBUS = 1.0f;
const float M_PI = std::atan(1) * 4;

// Tambahkan di bagian global, sebelum main()
GLfloat posisiSinar[] = { 15.0f, 15.0f, 15.0f, 1.0f };  // Posisi sumber cahaya
GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };  // Ambient light
GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Diffuse light
GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 0.5f };  // Specular light

// Enum untuk status permainan
enum StatusPermainan {
    LAYAR_AWAL,
    BERMAIN,
    GAME_OVER
};

// Struktur untuk menyimpan posisi 3D
struct Posisi3D {
    int x, y, z;
};

// Variabel global permainan
std::vector<Posisi3D> ular;
Posisi3D makanan;
int arahX = 1;
int arahY = 0;
StatusPermainan statusPermainan = LAYAR_AWAL;

// Variabel kamera
float sudutRotasiX = 89.0f;
float sudutRotasiY = 90.0f;
float jarakKamera = 25.0f;

// Struktur untuk tombol
struct Tombol {
    int x, y, lebar, tinggi;
};
Tombol tombolMulai = {
    LEBAR_JENDELA / 2 - 100,
    TINGGI_JENDELA / 2,
    200,
    50
};

// Fungsi untuk menghasilkan posisi acak (makanan)
Posisi3D hasilkanPosisiAcak() {
    Posisi3D pos;
    bool valid;
    do {
        valid = true;
        pos.x = rand() % JUMLAH_KOLOM;
        pos.y = rand() % JUMLAH_BARIS;
        pos.z = 0;  // Tetap di bidang dasar

        // Periksa tabrakan dengan ular
        for (const auto& bagian : ular) {
            if (pos.x == bagian.x && pos.y == bagian.y && pos.z == bagian.z) {
                valid = false;
                break;
            }
        }
    } while (!valid);

    return pos;
}

// Fungsi untuk mereset permainan
void resetPermainan() {
    ular.clear();
    ular.push_back({ JUMLAH_KOLOM / 2, JUMLAH_BARIS / 2, 0 });
    arahX = 1;
    arahY = 0;
    makanan = hasilkanPosisiAcak();
    statusPermainan = BERMAIN;
}

// Fungsi untuk menulis teks
void tulisTeks(void* fon, const char* teks, float x, float y) {
    glRasterPos2f(x, y);
    while (*teks) {
        glutBitmapCharacter(fon, *teks);
        teks++;
    }
}

// Fungsi untuk menggambar kubus
void gambarKubus(float x, float y, float z, float warna[3]) {
    // Material untuk objek
    GLfloat materialAmbient[] = { warna[0] * 0.3f, warna[1] * 0.3f, warna[2] * 0.3f, 1.0f };
    GLfloat materialDiffuse[] = { warna[0], warna[1], warna[2], 1.0f };
    GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat materialShininess[] = { 50.0f };

    // Atur material
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

    // Menggambar kubus
    glPushMatrix();
    glTranslatef(x, y, z);

    // Aktifkan bayangan
    glEnable(GL_POLYGON_OFFSET_FILL);
    glutSolidCube(UKURAN_KUBUS);

    glPopMatrix();
}

// Fungsi untuk menggambar layar awal
void gambarLayarAwal() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 10, 0, 0, 0, 0, 1, 0);

    glColor3f(1.0f, 1.0f, 1.0f);
    tulisTeks(GLUT_BITMAP_TIMES_ROMAN_24, "PERMAINAN ULAR 3D", -2, 1);

    // Teks tombol
    glColor3f(1.0f, 1.0f, 1.0f);
    tulisTeks(GLUT_BITMAP_HELVETICA_18, "Tekan Untuk Memulai", -1.5, -1);

    glutSwapBuffers();
}

// Fungsi untuk menggambar layar game over
void gambarLayarGameOver() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 10, 0, 0, 0, 0, 1, 0);

    glColor3f(1.0f, 0.0f, 0.0f);
    tulisTeks(GLUT_BITMAP_TIMES_ROMAN_24, "PERMAINAN BERAKHIR", -2, 2);

    char skorTeks[50];
    sprintf_s(skorTeks, "Panjang Ular: %lu", ular.size());
    glColor3f(1.0f, 1.0f, 1.0f);
    tulisTeks(GLUT_BITMAP_HELVETICA_18, skorTeks, -1, 0);

    // Teks tombol
    glColor3f(1.0f, 1.0f, 1.0f);
    tulisTeks(GLUT_BITMAP_HELVETICA_18, "Tekan Untuk Memulai Ulang", -1.5, -2);

    glutSwapBuffers();
}

// Fungsi untuk menggambar grid 3D
void gambarGrid3D() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Atur kamera dengan rotasi
    float posX = jarakKamera * cos(sudutRotasiY * M_PI / 180.0f) * cos(sudutRotasiX * M_PI / 180.0f);
    float posY = jarakKamera * sin(sudutRotasiX * M_PI / 180.0f);
    float posZ = jarakKamera * sin(sudutRotasiY * M_PI / 180.0f) * cos(sudutRotasiX * M_PI / 180.0f);

    gluLookAt(posX, posY, posZ, 0, 0, 0, 0, 1, 0);

    // Status game yang berbeda
    if (statusPermainan == LAYAR_AWAL) {
        gambarLayarAwal();
        return;
    }

    if (statusPermainan == GAME_OVER) {
        gambarLayarGameOver();
        return;
    }

    // Gambar grid
    float warnaAlas[3] = { 0.5f, 0.5f, 0.5f }; // warna abu abu
    for (int baris = 0; baris < JUMLAH_BARIS; baris++) {
        for (int kolom = 0; kolom < JUMLAH_KOLOM; kolom++) {
            gambarKubus(kolom - JUMLAH_KOLOM / 2.0f, -1.0f, baris - JUMLAH_BARIS / 2.0f, warnaAlas);
        }
    }

    // Gambar makanan
    float warnaMakanan[3] = { 1.0f, 0.0f, 0.0f }; // warna merah
    gambarKubus(makanan.x - JUMLAH_KOLOM / 2.0f, 0.0f, makanan.y - JUMLAH_BARIS / 2.0f, warnaMakanan);

    // Gambar ular
    float warnaUlar[3] = { 0.0f, 1.0f, 0.0f }; // warna hijau
    for (const auto& bagian : ular) {
        gambarKubus(bagian.x - JUMLAH_KOLOM / 2.0f, 0.0f, bagian.y - JUMLAH_BARIS / 2.0f, warnaUlar);
    }

    glutSwapBuffers();
}

// Fungsi timer (selalu loop)
void timer(int value) {
    if (statusPermainan != BERMAIN) {
        glutTimerFunc(200, timer, 0);
        return;
    }

    if (ular.empty()) {
        ular.push_back({ JUMLAH_KOLOM / 2, JUMLAH_BARIS / 2, 0 });
    }

    // Gerakkan badan ular (translasi)
    for (int i = ular.size() - 1; i > 0; i--) {
        ular[i] = ular[i - 1];
    }

    // Gerakkan kepala ular (translasi)
    ular[0].x += arahX;
    ular[0].y += arahY;

    // Cek posisi ular 
    if (ular[0].x >= JUMLAH_KOLOM) ular[0].x = 0;
    if (ular[0].x < 0) ular[0].x = JUMLAH_KOLOM - 1;
    if (ular[0].y >= JUMLAH_BARIS) ular[0].y = 0;
    if (ular[0].y < 0) ular[0].y = JUMLAH_BARIS - 1;

    // Periksa apakah terjadi tabrakan
    for (size_t i = 1; i < ular.size(); ++i) {
        if (ular[0].x == ular[i].x && ular[0].y == ular[i].y) {
            statusPermainan = GAME_OVER;
            break;
        }
    }

    // Periksa apakah ular makan (scalling)
    if (ular[0].x == makanan.x && ular[0].y == makanan.y) {
        Posisi3D ekorBaru = ular.back();
        ular.push_back(ekorBaru);
        makanan = hasilkanPosisiAcak();
    }

    glutPostRedisplay();
    glutTimerFunc(200, timer, 0);
}

// Fungsi tangani mouse
void tanganiMouse(int button, int state, int x, int y) {
    y = TINGGI_JENDELA - y;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (statusPermainan == LAYAR_AWAL || statusPermainan == GAME_OVER) {
            resetPermainan();
        }
    }
}

// Fungsi tangani keyboard
void tanganiKeyboard(unsigned char key, int x, int y) {
    if (statusPermainan != BERMAIN) return;

    switch (key) {

    // ubah bentuk ular dengan keyboard (morphing)
    case 'w':  // Ubah arah ke atas
        if (arahY != 1 || ular.size() == 1) {
            arahX = 0;
            arahY = -1;
        }
        break;
    case 's':  // Ubah arah ke bawah
        if (arahY != -1 || ular.size() == 1) {
            arahX = 0;
            arahY = 1;
        }
        break;
    case 'a':  // Ubah arah ke kiri
        if (arahX != 1 || ular.size() == 1) {
            arahX = -1;
            arahY = 0;
        }
        break;
    case 'd':  // Ubah arah ke kanan
        if (arahX != -1 || ular.size() == 1) {
            arahX = 1;
            arahY = 0;
        }
        break;

    case 'i':  // Rotasi ke atas
        sudutRotasiX += 5.0f;
        break;
    case 'k':  // Rotasi ke bawah
        sudutRotasiX -= 5.0f;
        break;
    case 'j':  // Rotasi ke kiri
        sudutRotasiY += 5.0f;
        break;
    case 'l':  // Rotasi ke kanan
        sudutRotasiY -= 5.0f;
        break;

    case '-':  // Zoom out
        if (jarakKamera < 50.0f) jarakKamera += 1.0f;
        break;
    case '=':  // Zoom in
        if (jarakKamera > 0.0f) jarakKamera -= 1.0f;
        break;

    case 'r':
        sudutRotasiX = 89.0f;
        sudutRotasiY = 90.0f;
        jarakKamera = 25.0f;
        break;
    }

    // Batasi sudut rotasi
    if (sudutRotasiX > 89.0f) sudutRotasiX = 89.0f;
    if (sudutRotasiX < -89.0f) sudutRotasiX = -89.0f;

    glutPostRedisplay();
}

// Fungsi utama
int main(int argc, char** argv) {
    // Inisialisasi GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(LEBAR_JENDELA, TINGGI_JENDELA);
    glutCreateWindow("Permainan Ular 3D");

    // Aktifkan pengujian kedalaman
    glEnable(GL_DEPTH_TEST);

    // Aktifkan smooth shading
    glShadeModel(GL_SMOOTH);  // Tambahkan baris ini untuk smooth shading

    // Aktifkan pencahayaan (shading)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Atur parameter cahaya
    glLightfv(GL_LIGHT0, GL_POSITION, posisiSinar);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // Aktifkan shadow mapping (tambahan)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    // Atur warna latar belakang hitam
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Atur proyeksi perspektif (grafik 3d)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)LEBAR_JENDELA / (float)TINGGI_JENDELA, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    // Inisialisasi random seed
    srand(time(NULL));

    // Daftarkan fungsi-fungsi callback
    glutDisplayFunc(gambarGrid3D);
    glutMouseFunc(tanganiMouse);
    glutKeyboardFunc(tanganiKeyboard);

    // Aktifkan timer untuk animasi
    glutTimerFunc(200, timer, 0);

    // Mulai loop utama
    glutMainLoop();

    return 0;
}