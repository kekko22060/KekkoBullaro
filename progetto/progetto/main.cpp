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

const int MAX_PROIETTILI = 9999999; // Numero massimo di proiettili gestiti contemporaneamente
float proiettiliX[MAX_PROIETTILI];
float proiettiliY[MAX_PROIETTILI];
float proiettiliDX[MAX_PROIETTILI];
float proiettiliDY[MAX_PROIETTILI];
float proiettiliAttivi[MAX_PROIETTILI];

// Tempo per gestire l'intervallo tra i proiettili
clock_t ultimoSparo = 0; // Tempo dell'ultimo sparo
const int intervalloSparo = 400; // 0,4 secondi

//funzione per verificare collisione tra 2 nemici
bool isCollisione(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
   
//funzione per far apparire a chermo "Game Over!"
void gameOver();

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

    int cuori = maxCuori;

    //spawn iniziale player
    int pX = (IMM2D_WIDTH - playerWidth) / 2;
    int pY = (IMM2D_HEIGHT - playerHeight) / 2;

    //spawn (casuale) iniziale nemici
    const int numNemici = 5;
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
    const int nSpeed = 3;  //velocità nemici
    const int mSpeed = 10;  //velocita sparo
    

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

            //risolve collisioni tra nemici
            for (int j = 0; j < numNemici; j++) {
                if (i != j && isCollisione(nX[i], nY[i], nemicoWidth, nemicoHeight, nX[j], nY[j], nemicoWidth, nemicoHeight)) {
                    if (nX[i] < nX[j]) nX[i] -= 1; else nX[i] += 1;
                    if (nY[i] < nY[j]) nY[i] -= 1; else nY[i] += 1;
                }
            }
        }

            // Sparo con intervallo 
            if (LeftMousePressed() && (clock() - ultimoSparo) >= intervalloSparo) {
                int pCenterX = pX + playerWidth / 2;
                int pCenterY = pY + playerHeight / 2;
                // Trova uno slot disponibile nell'array dei proiettili
                for (int i = 0; i < MAX_PROIETTILI; ++i) {
                    if (!proiettiliAttivi[i]) {
                        // Crea un nuovo proiettile
                        proiettiliX[i] = pCenterX;
                        proiettiliY[i] = pCenterY;

                        int mx = MouseX(), my = MouseY();
                        float angle = atan2(my - pCenterY, mx - pCenterX);
                        proiettiliDX[i] = cos(angle) * mSpeed;
                        proiettiliDY[i] = sin(angle) * mSpeed;
                        proiettiliAttivi[i] = true;

                        // Aggiorna il tempo dell'ultimo sparo
                        ultimoSparo = clock();
                        break; // Usa solo uno slot per volta
                    }
                }
            }
           

            // Aggiorna e disegna i proiettili
            for (int i = 0; i < MAX_PROIETTILI; ++i) {
                if (proiettiliAttivi[i]) {
                    proiettiliX[i] += proiettiliDX[i];
                    proiettiliY[i] += proiettiliDY[i];

                    DrawImage(proiettiliX[i], proiettiliY[i], sparo);

                    // Se il proiettile esce dallo schermo, lo disattiva
                    if (proiettiliX[i] < 0 || proiettiliX[i] > IMM2D_WIDTH || proiettiliY[i] < 0 || proiettiliY[i] > IMM2D_HEIGHT) {
                        proiettiliAttivi[i] = false;
                    }

                    // Controlla collisione proiettile-nemico
                    for (int j = 0; j < numNemici; ++j) {
                        if (isCollisione(proiettiliX[i], proiettiliY[i], ImageWidth(sparo), ImageHeight(sparo), nX[j], nY[j], nemicoWidth, nemicoHeight)) {
                            proiettiliAttivi[i] = false;

                            // Teletrasporta il nemico fuori dallo schermo
                            nX[j] = (rand() % 2 == 0 ? -nemicoWidth : IMM2D_WIDTH + nemicoWidth);
                            nY[j] = rand() % (IMM2D_HEIGHT + 2 * nemicoHeight) - nemicoHeight;
                        }
                    }
                }
                
                int nCenterX[numNemici];
                int nCenterY[numNemici];
                for (int i = 0; i < numNemici; ++i) {
                    nCenterX[i] = nX[i] + nemicoWidth / 2;
                    nCenterY[i] = nY[i] + nemicoHeight / 2;
            }

                for (int i = 0; i < numNemici; ++i) {
                    // Collisione tra player e nemico
                    if (isCollisione(pX, pY, playerWidth, playerHeight, nX[i], nY[i], nemicoWidth, nemicoHeight)) {
                        cuori--; // Decrementa i cuori
                        if (cuori <= 0) {
                            gameOver();
                        }

                        nX[i] = (rand() % 2 == 0 ? -nemicoWidth : IMM2D_WIDTH + nemicoWidth);
                        nY[i] = rand() % (IMM2D_HEIGHT + 2 * nemicoHeight) - nemicoHeight;
                    }
                }

                for (int i = 0; i < cuori; ++i) {
                    DrawImage(10 + i * (ImageWidth(cuore) + 5), 10, cuore);
                }


                //bordi dello schermo
                if (pX < 0) pX = 0;
                if (pX > IMM2D_WIDTH - playerWidth) pX = IMM2D_WIDTH - playerWidth;
                if (pY < 0) pY = 0;
                if (pY > IMM2D_HEIGHT - playerHeight) pY = IMM2D_HEIGHT - playerHeight;

                Present();

                Wait(10);
            }
    }
}


bool isCollisione(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    if (x1 + w1 < x2) {
        return false;
    }
    if (x1 > x2 + w2) {
        return false;
    }
    if (y1 + h1 < y2) {
        return false;
    }
    if (y1 > y2 + h2) {
        return false;
    }
    return true;
}


void gameOver() {
    Clear();

    int textHeight = 90;
    int x = IMM2D_WIDTH / 2;
    int y = (IMM2D_HEIGHT - textHeight) / 2;

    DrawString(x, y, "Game Over!", "Arial", 75, Red, true);

    Present();
    while (true) {
        Wait(100);
    }
}
