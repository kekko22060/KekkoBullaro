#define IMM2D_WIDTH 640
#define IMM2D_HEIGHT 480
#define IMM2D_SCALE 1

#define IMM2D_IMPLEMENTATION
#include "immediate2d.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

const int distMinNemici = 30;
const int maxCuori = 5;

// Struttura per gestire i proiettili
struct Proiettile {
    float x, y;
    float dx, dy; // Direzione del proiettile
    bool attivo;
};

const int MAX_PROIETTILI = 9999999; // Numero massimo di proiettili gestiti contemporaneamente
Proiettile proiettili[MAX_PROIETTILI];

// Tempo per gestire l'intervallo tra i proiettili
clock_t ultimoSparo = 0; // Tempo dell'ultimo sparo
const int intervalloSparo = 1000; // 1 secondo in millisecondi

//funzione per verificare collisione tra 2 nemici
bool isCollisione(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return !(x1 + w1 < x2 || x1 > x2 + w2 || y1 + h1 < y2 || y1 > y2 + h2);
}

void run() {
    UseDoubleBuffering(true);

    srand(time(NULL));

    Image player = LoadImage("player.png");
    Image cuore = LoadImage("cuore.png");
    Image nemico = LoadImage("chillguy_sinistra.png");
    Image sparo = LoadImage("matita.png");

    int playerWidth = ImageWidth(player);
    int playerHeight = ImageHeight(player);

    int nemicoWidth = ImageWidth(nemico);
    int nemicoHeight = ImageHeight(nemico);

    bool statoDelMouse = false;

    //spawn iniziale player
    int pX = (IMM2D_WIDTH - playerWidth) / 2;
    int pY = (IMM2D_HEIGHT - playerHeight) / 2;

    //spawn (casuale) iniziale nemici
    const int numNemici = 3;
    int nX[numNemici], nY[numNemici];
    for (int i = 0; i < numNemici; ++i) {
        bool collisione;
        do {
            collisione = false;
            nX[i] = rand() % (IMM2D_WIDTH - nemicoWidth);
            nY[i] = rand() % (IMM2D_HEIGHT - nemicoHeight);

            for (int j = 0; j < i; ++j) {
                if (isCollisione(nX[i], nY[i], nemicoWidth, nemicoHeight, nX[j], nY[j], nemicoWidth, nemicoHeight)) {
                    collisione = true;
                    break;
                }
            }
        } while (collisione);
    }

    const int pSpeed = 10;  //velocità player
    const int nSpeed = 1;  //velocità nemici
    const int mSpeed = 5;  //velocita sparo
    Proiettile proiettili[10]; // Array per gestire i proiettili
    int numProiettili = 0;

    while (true) {
        Clear();

        for (int i = 0; i < numNemici; ++i) {
            DrawImage(nX[i], nY[i], nemico);
        }

        DrawImage(pX, pY, player);

        for (int i = 0; i < maxCuori; ++i) {
            DrawImage(10 + i * (ImageWidth(cuore) + 5), 10, cuore);
        }

        //movimento player
        char key = LastKey();
        if (key == 'w') {
            pY -= pSpeed;
        }
        else if (key == 'a') {
            pX -= pSpeed;
        }
        else if (key == 's') {
            pY += pSpeed;
        }
        else if (key == 'd') {
            pX += pSpeed;
        }

        int pCenterX = pX + playerWidth / 2;
        int pCenterY = pY + playerHeight / 2;
        int xm = MouseX(), ym = MouseY();
        int mx = pCenterX, my = pCenterY;

        //movimento nemici
        for (int i = 0; i < numNemici; ++i) {
            if (nX[i] + nemicoWidth / 2 < pCenterX) {
                nX[i] += nSpeed;
            }
            if (nX[i] + nemicoWidth / 2 > pCenterX) {
                nX[i] -= nSpeed;
            }
            if (nY[i] + nemicoHeight / 2 < pCenterY) {
                nY[i] += nSpeed;
            }
            if (nY[i] + nemicoHeight / 2 > pCenterY) {
                nY[i] -= nSpeed;
            }

            // Sparo con intervallo di 1 secondo
            if (LeftMousePressed() && (clock() - ultimoSparo) >= intervalloSparo) {
                int pCenterX = pX + playerWidth / 2;
                int pCenterY = pY + playerHeight / 2;

                // Trova uno slot disponibile nell'array dei proiettili
                for (int i = 0; i < MAX_PROIETTILI; ++i) {
                    if (!proiettili[i].attivo) {
                        // Crea un nuovo proiettile
                        proiettili[i].x = pCenterX;
                        proiettili[i].y = pCenterY;

                        int mx = MouseX(), my = MouseY();
                        float angle = atan2(my - pCenterY, mx - pCenterX);
                        proiettili[i].dx = cos(angle) * mSpeed;
                        proiettili[i].dy = sin(angle) * mSpeed;
                        proiettili[i].attivo = true;

                        // Aggiorna il tempo dell'ultimo sparo
                        ultimoSparo = clock();
                        break; // Usa solo uno slot per volta
                    }
                }
            }
            //movimento del proiettile

            if (LeftMousePressed() && numProiettili < 10) {
                Proiettile p;
                p.x = pCenterX;   // Posizione di partenza del proiettile (al centro del player)
                p.y = pCenterY;
                int mx = MouseX(), my = MouseY();

                // Calcoliamo la direzione verso il mouse
                float angle = atan2(my - p.y, mx - p.x);
                p.dx = cos(angle) * mSpeed;
                p.dy = sin(angle) * mSpeed;
                p.attivo = true;

                // Aggiungiamo il proiettile all'array
                proiettili[numProiettili++] = p;
            }

            // Aggiorniamo e disegniamo i proiettili
            for (int i = 0; i < numProiettili; ++i) {
                if (proiettili[i].attivo) {
                    proiettili[i].x += proiettili[i].dx;
                    proiettili[i].y += proiettili[i].dy;

                    // Disegnamo il proiettile
                    DrawImage(proiettili[i].x, proiettili[i].y, sparo);

                    // Se il proiettile esce dallo schermo, lo disattiviamo
                    if (proiettili[i].x < 0 || proiettili[i].x > IMM2D_WIDTH || proiettili[i].y < 0 || proiettili[i].y > IMM2D_HEIGHT) {
                        proiettili[i].attivo = false;
                    }
                    // Controlla collisione proiettile-nemico
                    for (int j = 0; j < numNemici; ++j) {
                        if (isCollisione(proiettili[i].x, proiettili[i].y, ImageWidth(sparo), ImageHeight(sparo), nX[j], nY[j], nemicoWidth, nemicoHeight)) {
                            proiettili[i].attivo = false;

                            // Teletrasporta il nemico fuori dallo schermo
                            nX[j] = (rand() % 2 == 0 ? -nemicoWidth : IMM2D_WIDTH + nemicoWidth);
                            nY[j] = rand() % (IMM2D_HEIGHT + 2 * nemicoHeight) - nemicoHeight;
                        }
                    }
                }

                //risolve collisioni tra nemici
                for (int j = 0; j < numNemici; ++j) {
                    if (i != j && isCollisione(nX[i], nY[i], nemicoWidth, nemicoHeight, nX[j], nY[j], nemicoWidth, nemicoHeight)) {
                        if (nX[i] < nX[j]) nX[i] -= 1; else nX[i] += 1;
                        if (nY[i] < nY[j]) nY[i] -= 1; else nY[i] += 1;
                    }
                }
            }

            //bordi dello schermo
            if (pX < 0) pX = 0;
            if (pX > IMM2D_WIDTH - playerWidth) pX = IMM2D_WIDTH - playerWidth;
            if (pY < 0) pY = 0;
            if (pY > IMM2D_HEIGHT - playerHeight) pY = IMM2D_HEIGHT - playerHeight;

            Present();

            Wait(25);
        }


    }
}
