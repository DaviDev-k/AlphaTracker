#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "OCDFAInspection"
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"

#include <iostream>
#include <cstring>

#include "console.h"
#include "gui.h"

using namespace std;


void Gui::homeLoading(Settings set) {
    int y = 3;
    drawBox(15, y++, 28, 3);
    cout << c_bold << f_azr;
    coutxy("AlphaTracker", 29, y++, CENTER);
    cout << c_normal;
    drawBox(15, ++y, 28, 15);
    cout << c_faint;
    y += 2;
    for (int i = 0; i < 3; i++) {
        coutxy(set.sourceName[i], 29, y + 3 * i, CENTER);
        coutxy("...", 29, y + 1 + 3 * i, CENTER);
    }
    y += 10;
    cout << c_normal << f_ylw;
    coutxy("Impostazioni", 29, y, CENTER);
    cout << c_normal;
    coutxy("Premere [H] per mostrare i suggerimenti", 29, 23, CENTER);
    cout << c_normal << c_bold << f_azr;
    coutxy("[H]", 17, 23);
    cout << c_normal;
    gotoxy(1, 1);
}


void Gui::homeUpdate(Settings set, int idx, int source) {
    int y = 8;
    if (idx == 2 && set.isOpen[idx])
        coutxy(tab(100), 1, 1);
    cout << c_normal << (set.isOpen[idx] ? f_grn : c_faint);
    coutxy(set.sourceName[idx], 29, y + 3 * idx, CENTER);
    cout << (set.isOpen[idx] ? c_normal : c_faint);
    coutxy(set.genValue[source],
           29 + (idx == 1) * (strlen(set.genValue[source]) % 2),
           y + 1 + 3 * idx,
           CENTER);
    if (idx == 1)
        gotoxy(1, 1);
    cout << c_normal;
}


void Gui::homeSelect(Settings set, int idx) {
    int y = 8;
    for (int i = 0; i < 3; i++) {
        cout << c_normal << (set.isOpen[i] ? f_grn : c_faint);
        if (i == set.openSource[idx]) cout << c_reverse;
        coutxy(set.sourceName[i], 29, y + 3 * i, CENTER);
    }
    y += 10;
    cout << c_normal << f_ylw;
    if (set.openSource[idx] == 3) cout << c_reverse;
    coutxy("Impostazioni", 29, y, CENTER);
    cout << c_normal;
}


void Gui::settingsStable(Settings set) {
    int y = 2, j = 0;
    cout << c_bold << f_azr << c_uline;
    coutxy("IMPOSTAZIONI", 29, y++, CENTER);
    for (int i = 0; i < SET_GEN_NUM; i++) {
        if (i == 0 || i == 1 || i == 3) {
            cout << c_normal << f_grn;
            coutxy(set.sourceName[j++], 29, ++y, CENTER);
            cout << c_normal;
        }
        if (i != 4 && i != 6) y++;
        drawBox(set.genName[i],
                ((i != 4 && i != 6) ? 5 : 29), y,
                (i < 3 ? 47 : 23), 3, LEFT, f_ylw, NORMAL);
        if (i == 0 || i == 2) y += 3;
        if (i == 1 || i == 4) y += 2;
    }
    gotoxy(1, 1);
}


void Gui::settingsSelect(Settings set, int slc) {
    int y = 4, len, dim;
    char set_[256];
    for (int i = 0; i < SET_GEN_NUM; i++) {
        cout << f_ylw;
        if (slc == i) cout << c_reverse;
        if (i != 4 && i != 6) y++;
        else y--;
        gotoxy(((i != 4 && i != 6) ? 7 : 31), y);
        cout << ' ' << set.genName[i] << ' ' << c_normal;
        strcpy(set_, set.genValue[i]);
        len = strlen(set_);
        dim = (i < 3) ? 41 : 17;
        if (len > dim) {
            for (int j = 0; j < dim; j++)
                set_[j] = set_[j + len - dim];
            set_[dim] = '\0';
        }
        coutxy((len > dim ? '-' : ' '), ((i != 4 && i != 6) ? 7 : 31), ++y);
        cout << set_;
        if (i != 0) cout << c_blink << (slc == i ? bl_4 : " ") << ' ' << c_normal;
        if (i == 0 || i == 2) y += 3;
        if (i == 1 || i == 4) y += 1;
    }
    gotoxy(1, 1);
}


void Gui::guiStable(Settings set) {
    drawBox("View", 2, 2, 24, 11, CENTER, f_grn, BOLD);
    cout << f_ylw;
    for (int i = 0; i < SET_PRM_NUM_USER; i++)
        coutxy(set.prmName[i], 4, 4 + 2 * i);
    cout << c_normal;
    drawBox("Status", 2, 13, 24, 9, CENTER, f_grn, BOLD);
    cout << f_ylw;
    coutxy("Conteggio", 5, 15);
    coutxy("FPS", 5, 17);
    coutxy("Tempo", 5, 19);
    cout << c_normal;
    drawBox("Events", 27, 2, 28, 20, CENTER, f_grn, BOLD);
    cout << f_azr;
    coutxy("ID  frame  len  dir life", 29, 3);
    cout << c_normal;
    coutxy("Premere [H] per mostrare i suggerimenti", 29, 23, CENTER);
    cout << c_normal << c_bold << f_azr;
    coutxy("[H]", 17, 23);
    cout << c_normal;
}


void Gui::instructions() {
    cout << "\n " << c_bold << f_azr << c_uline
         << "ISTRUZIONI\n" << c_normal
         << "\n"
         << " L'interazione con il programma avviene unicamente da tastiera e solo quando\n"
         << " il focus e' sulla finestra con il video (quando la barra del titolo e' grigia);\n"
         << " se non si clicca all'esterno della finestra il programma procede in automatico\n"
         << "\n"
         << "\n" << c_bold << f_azr
         << " Comandi\n" << c_normal
         << "\n" << c_bold
         << "   Correzione della prospettiva\n" << c_normal
         << "     - [ESC]    termina l'esecuzione del programma\n"
         << "     - [ENTER]  seleziona i vertici attualmente visualizzati\n"
         << "\n" << c_bold
         << "   Tracciamento delle particelle\n" << c_normal
         << "     - [ENTER][ESC][SPACE]  terminano l'esecuzione del programma\n"
         << "     - [8][9]  scorrimento orizzontale fra i parametri\n"
         << "     - [5][6]  modificano il parametro selezionato\n"
         << "     - [4][7]  scorrimento verticale fra gli oggetti da mostrare\n"
         << "     - [1]     attiva/disattiva l'oggetto selezionato\n"
         << "     - [0]     ripristina i valori predefiniti\n"
         << "     - [+]     pausa/play\n\n";
}


#pragma clang diagnostic pop