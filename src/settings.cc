#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <SFML/Network.hpp>
#include "settings.h"

using namespace std;


void Settings::getSettings(char *setPath) {
    fstream myin;
    myin.open(setPath, ios::in);
    char bfr[256];

    myin.getline(bfr, 255, '\n');
    for (int i = 0; i < SET_GEN_NUM; i++) {
        myin.getline(genName[i], 255, '=');
        myin.getline(genValue[i], 255, '\n');
    }
    myin.getline(bfr, 255, '\n');
    myin.getline(bfr, 255, '\n');
    {
        myin.getline(bfr, 255, '=');
        myin.getline(savePath, 255, '\n');
    }
    myin.getline(bfr, 255, '\n');
    myin.getline(bfr, 255, '\n');
    for (int i = 0; i < SET_RES_NUM; i++) {
        myin.getline(resName[i], 255, '=');
        myin.getline(bfr, 255, 'x');
        resWidth[i] = atoi(bfr);
        myin.getline(bfr, 255, '\n');
        resHeight[i] = atoi(bfr);
    }
    myin.getline(bfr, 255, '\n');
    myin.getline(bfr, 255, '\n');
    for (int i = 0; i < SET_PRM_NUM; i++) {
        myin.getline(prmName[i], 255, '=');
        myin.getline(bfr, 8, '\n');
        prmDefault[i] = atoi(bfr);
    }
    myin.getline(bfr, 255, '\n');
    myin.getline(bfr, 255, '\n');
    for (int i = 0; i < SET_VAR_NUM; i++) {
        myin.getline(varName[i], 255, '=');
        myin.getline(bfr, 8, '\n');
        var[i] = atoi(bfr);
    }
    myin.getline(bfr, 255, '\n');
    myin.getline(bfr, 255, '\n');
    for (int i = 0; i < 1; i++) {
        myin.getline(bfr, 255, '=');
        myin.getline(bfr, 2, '\n');
        SEC_FRAME = atoi(bfr);
    }

    myin.close();
}


void Settings::getValues() {
    strcpy(videoInPath, genValue[s1_folder]);
    strcat(videoInPath, genValue[s1_file]);
    strcpy(ipcLink, "http://");
    if (strcmp(genValue[s2_user], "") != 0) {
        strcat(ipcLink, genValue[s2_user]);
        strcat(ipcLink, ":");
        strcat(ipcLink, genValue[s2_password]);
        strcat(ipcLink, "@");
    }
    strcat(ipcLink, genValue[s2_address]);
    strcat(ipcLink, ":");
    strcat(ipcLink, genValue[s2_port]);
    strcat(ipcLink, "/video?x.mjpeg");
    // camera = strcmp(setValue[s0_camera], "Pi Camera V2");
    port = atoi(genValue[s2_port]);
}



bool isOnline(Settings set) {
    mutex m;
    condition_variable cv;
    bool ret;

    thread t([&cv, &ret, &set](char *adr, unsigned short prt) {
        ret = isOnline_(set.genValue[s2_address], set.port);
        cv.notify_one();
    }, set.genValue[s2_address], set.port);

    t.detach();

    unique_lock<mutex> l(m);
    if (cv.wait_for(l, chrono::duration<int>(1)) == cv_status::timeout)
        throw runtime_error("Timeout");

    return ret;
}

bool isOnline_(char *address, int port) {
    sf::TcpSocket socket;
    bool open = (socket.connect(sf::IpAddress(address), port) == sf::Socket::Done);
    socket.disconnect();
    return open;
}