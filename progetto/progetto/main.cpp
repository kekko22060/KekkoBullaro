#define IMM2D_WIDTH 640
#define IMM2D_HEIGHT 480
#define IMM2D_SCALE 1

#define IMM2D_IMPLEMENTATION
#include "immediate2d.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>

using namespace std;

const int maxCuori = 5;
const int maxProiettili = 10;

const int pSpeed = 10;  // Velocità player
const int nSpeed = 1;   // Velocità nemici
const int mSpeed = 10;  // Velocità proiettile
const int intervalloSparo = 400; // Intervallo sparo in millisecondi
const int punteggioVittoria = 100; // Punteggio necessario per vincere

// Funzione per verificare collisione
bool isCollisione(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
// Funzione per far apparire "Game Over!"
void gameOver();
// Funzione per far apparire "Hai vinto!"
void haiVinto();

void menuIniziale() {
    while (true) {
        Clear();
        DrawString(IMM2D_WIDTH / 2 - 25, IMM2D_HEIGHT / 2 - 50, "Premi Invio per iniziare", "Arial", 30, White, true);
        Present();

        char key = LastKey();
        if (key == '\r' ) { // Premendo il tasto invio si avvia il gioco
            break;
        }

        Wait(25);
    }
}

void run() {
    UseDoubleBuffering(true);
    srand(time(NULL));

    // Mostra il menu iniziale
    menuIniziale();

    Image player = LoadImage("player.png");
    Image cuore = LoadImage("cuore.png");
    Image nemico = LoadImage("chillguy_sinistra.png");
    Image sparo = LoadImage("matita.png");

    int playerWidth = ImageWidth(player);
    int playerHeight = ImageHeight(player);

    int nemicoWidth = ImageWidth(nemico);
    int nemicoHeight = ImageHeight(nemico);

    int cuori = maxCuori;
    int punteggio = 0;

    // Spawn iniziale player
    int pX = (IMM2D_WIDTH - playerWidth) / 2;
    int pY = (IMM2D_HEIGHT - playerHeight) / 2;

    // Spawn iniziale nemici
    const int numNemici = 5;
    int nX[numNemici], nY[numNemici];
    for (int i = 0; i < numNemici; ++i) {
        nX[i] = rand() % (IMM2D_WIDTH - nemicoWidth);
        nY[i] = rand() % (IMM2D_HEIGHT - nemicoHeight);
    }

    // Gestione proiettili
    float pXArr[maxProiettili] = { 0 };
    float pYArr[maxProiettili] = { 0 };
    float pDxArr[maxProiettili] = { 0 };
    float pDyArr[maxProiettili] = { 0 };
    bool pAttivo[maxProiettili] = { false };

    clock_t ultimoSparo = 0;

    while (true) {
        Clear();

        // Disegna nemici
        for (int i = 0; i < numNemici; ++i) {
            DrawImage(nX[i], nY[i], nemico);
        }

        // Disegna player
        DrawImage(pX, pY, player);

        // Movimento player
        char key = LastKey();
        if (key == 'w')
            pY -= pSpeed;
        if (key == 'a')
            pX -= pSpeed;
        if (key == 's')
            pY += pSpeed;
        if (key == 'd')
            pX += pSpeed;

        // Movimento nemici verso il player
        int pCenterX = pX + playerWidth / 2;
        int pCenterY = pY + playerHeight / 2;
        for (int i = 0; i < numNemici; ++i) {
            if (nX[i] + nemicoWidth / 2 < pCenterX)
                nX[i] += nSpeed;
            if (nX[i] + nemicoWidth / 2 > pCenterX)
                nX[i] -= nSpeed;
            if (nY[i] + nemicoHeight / 2 < pCenterY)
                nY[i] += nSpeed;
            if (nY[i] + nemicoHeight / 2 > pCenterY)
                nY[i] -= nSpeed;
        }

        // Sparo
        if (LeftMousePressed() && (clock() - ultimoSparo) >= intervalloSparo) {
            for (int i = 0; i < maxProiettili; ++i) {
                if (!pAttivo[i]) {
                    int mx = MouseX(), my = MouseY();
                    float angle = atan2(my - pCenterY, mx - pCenterX);
                    pXArr[i] = pCenterX;
                    pYArr[i] = pCenterY;
                    pDxArr[i] = cos(angle) * mSpeed;
                    pDyArr[i] = sin(angle) * mSpeed;
                    pAttivo[i] = true;
                    ultimoSparo = clock();
                    break;
                }
            }
        }

        // Movimento proiettili e gestione collisioni
        for (int i = 0; i < maxProiettili; ++i) {
            if (pAttivo[i]) {
                pXArr[i] += pDxArr[i];
                pYArr[i] += pDyArr[i];
                DrawImage(pXArr[i], pYArr[i], sparo);

                if (pXArr[i] < 0 || pXArr[i] > IMM2D_WIDTH || pYArr[i] < 0 || pYArr[i] > IMM2D_HEIGHT) {
                    pAttivo[i] = false;
                }

                for (int j = 0; j < numNemici; ++j) {
                    if (isCollisione(pXArr[i], pYArr[i], ImageWidth(sparo), ImageHeight(sparo), nX[j], nY[j], nemicoWidth, nemicoHeight)) {
                        pAttivo[i] = false;
                        nX[j] = rand() % IMM2D_WIDTH;
                        nY[j] = rand() % IMM2D_HEIGHT;
                        punteggio += 10; // Incrementa punteggio
                        if (punteggio >= punteggioVittoria) {
                            haiVinto();
                        }
                    }
                }
            }
        }

        // Collisione player-nemico + GameOver
        for (int i = 0; i < numNemici; ++i) {
            if (isCollisione(pX, pY, playerWidth, playerHeight, nX[i], nY[i], nemicoWidth, nemicoHeight)) {
                cuori--;

                if (cuori <= 0) {
                    gameOver();
                }

                // Teletrasporto del nemico fuori dai bordi della schermata
                if (rand() % 2 == 0) {
                    if (rand() % 2 == 0)
                        nX[i] = -nemicoWidth;
                    else
                        nX[i] = IMM2D_WIDTH + nemicoWidth;
                }
                else {
                    if (rand() % 2 == 0)
                        nY[i] = -nemicoHeight;
                    else
                        nY[i] = IMM2D_HEIGHT + nemicoHeight;
                }
            }
        }


        // Disegna cuori
        for (int i = 0; i < cuori; ++i) {
            DrawImage(10 + i * (ImageWidth(cuore) + 5), 10, cuore);
        }

        // Disegna punteggio
        string punteggioStr = "Punteggio: " + to_string(punteggio);
        DrawString(10, 50, punteggioStr.c_str(), "Arial", 20, White);

        // Bordi dello schermo
        if (pX < 0)
            pX = 0;
        if (pX > IMM2D_WIDTH - playerWidth)
            pX = IMM2D_WIDTH - playerWidth;
        if (pY < 0)
            pY = 0;
        if (pY > IMM2D_HEIGHT - playerHeight)
            pY = IMM2D_HEIGHT - playerHeight;

        Present();
        Wait(25);
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
    DrawString(IMM2D_WIDTH / 2 - 150, IMM2D_HEIGHT / 2 - 50, "Game Over!", "Arial", 75, Red, true);
    Present();
    while (true) {
        Wait(100);
    }
}
void haiVinto() {
    Clear();
    DrawString(IMM2D_WIDTH / 2 - 25, IMM2D_HEIGHT / 2 - 50, "Hai vinto!", "Arial", 75, Green, true);
    Present();
    while (true) {
        Wait(100);
    }
}
