#ifndef SETTINGS_H
#define SETTINGS_H

#include <fstream>

const int SET_GEN_NUM = 7;
const int SET_RES_NUM = 5;
const int SET_PRM_NUM = 7;
const int SET_VAR_NUM = 10;
const int SET_PRM_NUM_USER = 4;


struct Settings {

    // Sources
    bool isOpen[3];
    int openSource[4], openNum;

    // Generals
    char genName[SET_GEN_NUM][256], genValue[SET_GEN_NUM][256];
    char videoInPath[256], ipcLink[256], savePath[256];
    int camera, port;
    const char sourceName[3][16] = {"FOTOCAMERA",
                                    "FILE VIDEO",
                                    "IP CAMERA"};

    // Resolutions
    char resName[SET_RES_NUM][256];
    int resWidth[SET_RES_NUM], resHeight[SET_RES_NUM];

    // Parameters
    char prmName[SET_PRM_NUM][256];
    int prmDefault[SET_PRM_NUM];

    // Variables
    char varName[SET_VAR_NUM][256];
    int var[SET_VAR_NUM];

    // Show
    int SEC_FRAME;

    // Methods
    void getSettings(char *setPath);

    void getValues();

};


enum generals {
    s0_camera,
    s1_folder,
    s1_file,
    s2_address,
    s2_port,
    s2_user,
    s2_password
};

enum sources {
    CAM,
    VID,
    IPC,
    SET
};

enum resolutions {
    RES_SCREEN = 3,
    RES_OUT = 4
};

enum parameters {
    MSK,
    BAK,
    RAY,
    NUM,
    ELP,
    SEG,
    CON
};

enum variables {
    FPS,
    DELAY,
    AREA_PERC,
    DIR_DIFF,
    MIN_LIFE,
    LEN_MULT,
    BLUR_VL,
    THR_VL,
    VOID_FRM,
    VOID_AREA
};


bool isOnline(Settings set);

bool isOnline_(char *address, int port);


#endif // SETTINGS_H