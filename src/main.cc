#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

#include <opencv2/opencv.hpp>
#include <opencv2/bgsegm.hpp>
#include <iostream>
#include <cstring>
#include <cmath>

#include "gui.h"
#include "stack.h"
#include "console.h"
#include "settings.h"
#include "functions.h"

#define INIT_MAT Mat(OUT_RES, OUT_RES, CV_8U, Scalar(0))
#define GUI true
#define DEBUG false

using namespace std;
using namespace cv;

// FIXME camera source

int main(int argc, char **argv) {

/**** [ Initialize ] ****/

    if (argc != 2) {
        cerr << "Usage: ./cmake-build-debug/main \"$PWD/\"\n";
        exit(0);
    }

    // Common
    const char *PWD = argv[1];
    char chr, bfr[256];
    int select, key;

    // Console
    streambuf *cerrSbuf = cerr.rdbuf();
    ofstream ferr("/dev/null");
    cerr.rdbuf(ferr.rdbuf());
    nocursor();
    noecho();




/**** [ Settings ] ****/

    // KB event window
    namedWindow("KBevent", WINDOW_GUI_NORMAL);
    resizeWindow("KBevent", 0, 0);
    moveWindow("KBevent", 4000, 4000);

    // Init settings
    char setPath[256];
    strcpy(setPath, PWD);
    strcat(setPath, "settings.txt");
    Gui gui;
    Settings set;
    set.getSettings(setPath);

    // Edit settings
    bool restart;
    do {
        restart = false;
        set.getValues();

        // Home
        system("clear");
        gui.homeLoading(set);
        set.isOpen[CAM] = VideoCapture(0 /*camera*/).isOpened();
        gui.homeUpdate(set, CAM, s0_camera);
        set.isOpen[VID] = VideoCapture(set.videoInPath).isOpened();
        gui.homeUpdate(set, VID, s1_file);
        try { set.isOpen[IPC] = isOnline(set); }
        catch (runtime_error &e) { set.isOpen[IPC] = false; }
        gui.homeUpdate(set, IPC, s2_address);

        // Open sources
        set.openNum = 0;
        for (int i = 0; i < 3; i++) {
            if (set.isOpen[i])
                set.openSource[set.openNum++] = i;
        }
        set.openSource[set.openNum] = SET;
        select = 0;

        // Select source
        key = -1;
        while (key != KEY_ENTER && key != KEY_ENTER_ALT) {
            gui.homeSelect(set, select);
            key = waitKeyEx(0);
            switch (key) {
                case KEY_DOWN:
                    select = (select + 1) % (set.openNum + 1);
                    break;
                case KEY_UP:
                    select = (select == 0) ? (set.openNum) : (select - 1);
                    break;
                case KEY_ESC:
                    quit(0);
                    break;
            }
        }
        system("clear");

        // Edit settings
        if (set.openSource[select] == SET) {
            gui.settingsStable(set);
            select = 0;
            key = -1;
            bool saved = false, shift = false, caps = false;

            while (key != KEY_ENTER && key != KEY_ESC) {
                gui.settingsSelect(set, select);

                if (saved)
                    saved = false;
                else
                    key = waitKeyEx(0);
                switch (key) {
                    case KEY_DOWN:
                        select = (select + 1 + (select >= 3 && select <= 5)) % SET_GEN_NUM;
                        break;
                    case KEY_UP:
                        select = (select == 0) ? 5 : (select - 1 - (select >= 4 && select <= 6));
                        break;
                    case KEY_RIGHT:
                    case KEY_TAB:
                        select = (select + 1) % SET_GEN_NUM;
                        break;
                    case KEY_LEFT:
                        select = (select == 0) ? (SET_GEN_NUM - 1) : (select - 1);
                        break;
                    default:
                        while (!saved && select != 0) {
                            if (key == KEY_ENTER || key == KEY_ESC || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT ||
                                key == KEY_UP)
                                saved = true;
                            else {
                                int len = strlen(set.genValue[select]);
                                if (key == KEY_BACK)
                                    set.genValue[select][len - 1] = '\0';
                                else if (key == KEY_SHIFT || key == KEY_SHIFT_ALT)
                                    shift = !shift;
                                else if (key == KEY_CAPS) {
                                    caps = !caps;
                                    shift = caps;
                                } else {
                                    set.genValue[select][len] = char_case(key, shift);
                                    set.genValue[select][len + 1] = '\0';
                                    shift = caps;
                                }
                                gui.settingsSelect(set, select);
                                // coutxy (key, 1, 1);
                                key = waitKeyEx(0);
                                // coutxy (tab(8), 1, 1);
                            }
                        }
                        break;
                }
            }
            if (saved) {
                fstream myout;
                myout.open(setPath, ios::out);
                myout << "#GENERALS\n";
                for (int i = 0; i < SET_GEN_NUM; i++)
                    myout << set.genName[i] << '=' << set.genValue[i] << endl;
                myout << "\n#SAVING"
                      << "\nsave_path=" << set.savePath << endl;
                myout << "\n#RESOLUTIONS\n";
                for (int i = 0; i < SET_RES_NUM; i++)
                    myout << set.resName[i] << '=' << set.resWidth[i] << 'x' << set.resHeight[i] << endl;
                myout << "\n#PARAMETERS\n";
                for (int i = 0; i < SET_PRM_NUM; i++)
                    myout << set.prmName[i] << '=' << set.prmDefault[i] << endl;
                myout << "\n#VARIABLES\n";
                for (int i = 0; i < SET_VAR_NUM; i++)
                    myout << set.varName[i] << '=' << set.var[i] << endl;
                myout << "\n#SHOW\n";
                for (int i = 0; i < 1; i++)
                    myout << "SEC_FRAME=" << set.SEC_FRAME << endl;
                myout.close();
            }
            restart = true;
        }
    } while (restart);
    destroyAllWindows();

    // Store settings
    const int SOURCE = set.openSource[select];
    const int SCREEN_W = set.resWidth[RES_SCREEN];
    const int SCREEN_H = set.resHeight[RES_SCREEN];
    const int INPUT_W = set.resWidth[SOURCE];
    const int INPUT_H = set.resHeight[SOURCE];
    const int SOURCE_W = SCREEN_W / 2;  /*const int SOURCE_W = SCREEN_H - 68;*/
    const int SOURCE_H = SOURCE_W * INPUT_H / INPUT_W;
    const int OUT_RES = set.resWidth[RES_OUT];
    const int CROP_L = INPUT_H / 4;
    const int OUTPUT_L = SOURCE_W;
    const int MIN_LEN = OUT_RES / 30;




/**** [ Video source ] ****/

    // Video input
    VideoCapture cap;
    int totFrame, totTime;
    switch (SOURCE) {
        case CAM:
            cap = VideoCapture(set.camera);
            break;
        case VID:
            cap = VideoCapture(set.videoInPath);
            totFrame = (int) cap.get(CAP_PROP_FRAME_COUNT);
            totTime = (int) (totFrame / cap.get(CAP_PROP_FPS));
            break;
        case IPC:
            cap = VideoCapture(set.ipcLink);
            break;
    }

    // ERROR
    if (!cap.isOpened()) {
        cap.release();
        cerr.rdbuf(cerrSbuf);
        cerr << f_red;
        switch (SOURCE) {
            case CAM:
                cerr << "\n Impossibile aprire la fotocamera " << set.genValue[s0_camera] << endl;
                cap = VideoCapture(0);
                if (cap.isOpened())
                    cerr << f_grn << "\n Fotocamera 0 aperta correttamente\n\n";
                else {
                    cerr << f_red << "\n Impossibile aprire la fotocamera 0\n" << f_ylw
                         << "\n Verificare che:\n"
                         << "  - il dispositivo sia funzionante\n"
                         << "  - il dispositivo sia collegato correttamente";
                }
                break;
            case VID:
                cerr << " Impossibile aprire il file video\n" << f_ylw
                     << "\n Verificare che:\n"
                     << "  - il percorso \"" << set.videoInPath << "\" sia corretto\n"
                     << "  - il file sia leggibile";
                break;
            case IPC:
                cerr << " Impossibile raggiungere IP camera\n"
                     << f_ylw << "\n Verificare che:\n"
                     << "  - il dispositivo sia attivo\n"
                     << "  - il dispositivo sia connesso alla stessa rete del Raspberry Pi\n"
                     << "  - l'indirizzo \"" << set.ipcLink << "\" sia corretto";
                break;
        }

        cerr << c_normal;
        if (!cap.isOpened()) {
            pause();
            quit(SOURCE);
        }
    }

    // Set file video resolution
    if (SOURCE == VID) {
        cap.set(CAP_PROP_FRAME_HEIGHT, INPUT_H);
        cap.set(CAP_PROP_FRAME_WIDTH, INPUT_W);
    }




/**** [ Perspective ] ****/

    // Window
    namedWindow("Prospettiva", WINDOW_GUI_NORMAL);
    resizeWindow("Prospettiva", SOURCE_W, SOURCE_H);
    moveWindow("Prospettiva", SCREEN_W / 2, SCREEN_H - SOURCE_H);

    // Instructions
    gotoxy(1, 1);
    gui.instructions();

    // Perspective loop
    Mat source, cropped, dst, transmtx;
    bool found = false;
    key = -1;
    for (int f = 1; ((key = waitKey(1 + (SOURCE == VID) * set.var[DELAY])) != KEY_ENTER
                     && key != KEY_ENTER_ALT) || !found; f++) {

        if (key == KEY_ESC) {
            cap.release();
            quit(3);
        }

        cap.read(source);

        // ERROR
        if (source.empty()) {
            destroyAllWindows();
            cap.release();
            system("clear");
            cerr.rdbuf(cerrSbuf);
            cerr << f_red << endl;
            switch (SOURCE) {
                case CAM:
                    cerr << " Impossibile comunicare con la fotocamera " << set.genValue[s0_camera] << endl
                         << f_ylw << "\n Verificare che:\n"
                         << "  - il dispositivo sia funzionante\n"
                         << "  - il dispositivo sia collegato correttamente";
                    break;
                case VID:
                    if (f - 1 == totFrame)
                        cerr << f_ylw << " Video finito al frame " << f - 1;
                    else
                        cerr << " Impossibile riprodurre il video, file danneggiato al frame " << f;
                    break;
                case IPC:
                    cerr << " Impossibile comunicare con IP camera\n"
                         << f_ylw << "\n Verificare che:\n"
                         << "  - il dispositivo sia attivo\n"
                         << "  - il dispositivo sia connesso alla stessa rete del Raspberry Pi";
                    break;
            }
            pause();
            quit(4 + SOURCE);
        }

        resize(source, source, Size(INPUT_W, INPUT_H), 0, 0);
        if (SOURCE == CAM)
            flip(source, source, -1);

        // Find quad
        if (!found && findQuad(source, transmtx, OUT_RES))
            found = true;
        if (found) {
            findQuad(source, transmtx, OUT_RES);
            warpPerspective(source, cropped, transmtx, Size(OUT_RES, OUT_RES));
            resize(cropped, dst, Size(CROP_L, CROP_L));
            dst.copyTo(source(Rect(0, 0, CROP_L, CROP_L)));
            rectangle(source, Point(0, 0), Point(CROP_L, CROP_L), Scalar(20, 255, 0), 1, 8);
        }
        imshow("Prospettiva", source);
    }
    destroyAllWindows();
    system("clear");




/**** [ Init stuff ] ****/

    // Matrix
    const int MAT = 4;
    const int MAT2 = 6;
    enum imgOutput {
        MASK, BLUR, BLOB, BLNK,
        PRV, OLD, NEW, DED, BKG, RGB
    };
    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(1, 1));
    Mat kernel9 = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
    Mat frame, crop, gray, mat[MAT + MAT2];
    for (auto &mat_ : mat) mat_ = INIT_MAT;

    // Parameters
    struct Parameter {
        int min, max;
        bool txt;
        string str[MAT];
        int value;
    };
    Parameter prm[SET_PRM_NUM];
    prm[MSK] = {0, MAT - 1, true, {"originale", "sfocata  ", "smussata ", "vuota    "}};
    prm[BAK] = {0, 10, false};
    prm[RAY] = {0, 1, true, {OFF, ON}};
    prm[NUM] = {0, 1, true, {OFF, ON}};
    prm[ELP] = {0, 1, true, {OFF, ON}};
    prm[SEG] = {0, 1, true, {OFF, ON}};
    prm[CON] = {0, 1, true, {OFF, ON}};
    for (int i = 0; i < SET_PRM_NUM; i++)
        prm[i].value = set.prmDefault[i];

    // Tracks
    Ptr<BackgroundSubtractor> BackSub = createBackgroundSubtractorMOG2(80, 24, false);
    int voidFrames = -1;
    int contourSize;
    int **joint;
    int *srt;
    int id;
    int counter = 0;
    int aliveSize;
    int alive[32];
    int aliveNew[32];
    alive[0] = -1;
    aliveNew[0] = -1;
    track ray;
    stack s;
    init(s);

    // Output
    namedWindow("Tracking", WINDOW_GUI_NORMAL);
    resizeWindow("Tracking", OUTPUT_L, OUTPUT_L);
    moveWindow("Tracking", SCREEN_W / 2, 36);
    char videoSourceName[256];
    char videoSourcePath[256];
    strcpy(videoSourceName, "crop_");
    strcat(videoSourceName, currentDateTime());
    strcat(videoSourceName, ".mp4");
    strcpy(videoSourcePath, set.savePath);
    strcat(videoSourcePath, videoSourceName);
    VideoWriter videoSource(videoSourcePath,
                            VideoWriter::fourcc('a', 'v', 'c', '1'),
                            set.var[FPS],
                            Size(OUT_RES, OUT_RES));
    char videoCropName[256];
    char videoCropPath[256];
    strcpy(videoCropName, "source_");
    strcat(videoCropName, currentDateTime());
    strcat(videoCropName, ".mp4");
    strcpy(videoCropPath, set.savePath);
    strcat(videoCropPath, videoCropName);
    VideoWriter videoCrop(videoCropPath,
                          VideoWriter::fourcc('a', 'v', 'c', '1'),
                          set.var[FPS],
                          Size(INPUT_W, INPUT_H));

    // Interface
    select = 0;
    bool frameByFrame = false;
    int out;
    if (GUI) {
        system("clear");
        gui.guiStable(set);
    }

    // Time
    bool GO = true;
    key = -1;
    TickMeter tm;
    tm.start();
    double time, time0;
    int fps;
    if (SOURCE == VID)
        cap = VideoCapture(set.videoInPath);
    else
        time0 = cap.get(CAP_PROP_POS_MSEC) / 1000;




/**** [ Video loop ] ****/

    for (int f = 1; GO; f++) {

        cap.read(frame);

        // ERROR
        if (frame.empty()) {
            destroyAllWindows();
            gotoxy(1, 22);
            cerr.rdbuf(cerrSbuf);
            cerr << f_red << endl;
            switch (SOURCE) {
                case CAM:
                    cerr << " Impossibile comunicare con la fotocamera " << set.genValue[s0_camera] << endl
                         << f_ylw << "\n Verificare che:\n"
                         << "  - il dispositivo sia funzionante\n"
                         << "  - il dispositivo sia collegato correttamente";
                    break;
                case VID:
                    if (f - 1 == totFrame)
                        cerr << f_ylw << " Video finito al frame " << f - 1;
                    else
                        cerr << " Impossibile riprodurre il video, file danneggiato al frame " << f;
                    break;
                case IPC:
                    cerr << " Impossibile comunicare con IP camera\n"
                         << f_ylw << "\n Verificare che:\n"
                         << "  - il dispositivo sia attivo\n"
                         << "  - il dispositivo sia connesso alla stessa rete del Raspberry Pi";
                    break;
            }
            cerr << c_normal;
            GO = false;
        }

        // Quit keyboard
        if (GO
            && key == KEY_ESC
            || key == KEY_SPACE
            || key == KEY_ENTER
            || key == KEY_ENTER_ALT) {
            destroyAllWindows();
            gotoxy(1, 22);
            cout << f_ylw << " Fermato al frame " << f - 1 << c_normal;
            GO = false;
        }

        // Here comes the magic
        if (GO) {

            /**** [ KB events ] ****/

            // Interact
            if (key != -1) {
                switch (key) {
                    // Restore
                    case '0':
                    case 176 + 0:
                    case 'r':
                    case 'R':
                        for (int i = 0; i < SET_PRM_NUM_USER; i++)
                            prm[i].value = set.prmDefault[i];
                        break;

                        // change prm_name
                    case '5':
                    case 176 + 5:
                    case KEY_LEFT:
                        if (prm[select].txt)
                            prm[select].value = (prm[select].value > prm[select].min)
                                                ? prm[select].value - 1 : prm[select].max;
                        else
                            prm[select].value -= (prm[select].value > prm[select].min);
                        break;
                    case '6':
                    case 176 + 6:
                    case KEY_RIGHT:
                        if (prm[select].txt)
                            prm[select].value = (prm[select].value + 1) % (prm[select].max + 1);
                        else
                            prm[select].value += (prm[select].value < prm[select].max);
                        break;

                        // vertcal selector
                    case '7':
                    case 176 + 7:
                    case KEY_UP:
                        select--;
                        if (select < 0) select = SET_PRM_NUM_USER - 1;
                        break;
                    case '4':
                    case 176 + 4:
                    case KEY_DOWN:
                        select = (select + 1) % SET_PRM_NUM_USER;
                        break;

                        // pause
                    case '+':
                    case 171:
                        if (GUI) {
                            gotoxy(21, 9);
                            cout << f_vlt << ln_v << ln_v << c_normal;
                        }
                        frameByFrame = true;
                        tm.stop();
                        break;
                }
            }

            // WaitKey
            if (frameByFrame) {
                if (GUI) {
                    gotoxy(21, 9);
                    cout << f_vlt << ln_v << ln_v << c_normal;
                }
                key = waitKeyEx();
                if (key == '+' || key == 171) {
                    frameByFrame = false;
                    if (GUI) coutxy("  ", 21, 9);
                }
            }
            tm.start();
            key = waitKeyEx(set.var[DELAY]);


            /**** [ Process frame ] ****/

            // Perspective
            resize(frame, frame, Size(INPUT_W, INPUT_H));
            if (SOURCE == CAM)
                flip(frame, frame, -1);
            warpPerspective(frame, crop, transmtx, Size(OUT_RES, OUT_RES));

            // Mask
            cvtColor(crop, gray, COLOR_BGR2GRAY);
            // imshow("crop",gray);
            BackSub->apply(gray, mat[MASK]);
            rectangle(mat[MASK], Point(0, 0), Point(OUT_RES, OUT_RES), Scalar(0), 2 * MIN_LEN, 8);  // erase borders
            dilate(mat[MASK], mat[MASK], kernel1, Point(-1, -1), 4);
            erode(mat[MASK], mat[MASK], kernel1, Point(-1, -1), 4);


            /**** [ Critical change ] ****/

            if (voidFrames >= 0)
                voidFrames--;

            if (!(voidFrames < 0 && countNonZero(mat[MASK]) < (OUT_RES * OUT_RES) / set.var[VOID_AREA])) {

                /**** [ Pause analysis ] ****/

                if (voidFrames < 0 || countNonZero(mat[MASK]) > (OUT_RES * OUT_RES) / set.var[VOID_AREA])
                    voidFrames = set.var[VOID_FRM];

                if (prm[BAK].value > 0) {
                    cvtColor(mat[prm[MSK].value], mat[RGB], COLOR_GRAY2BGR);
                    mat[BKG] = crop * prm[BAK].value / 10 + mat[RGB] * (10 - prm[BAK].value) / 10;
                    out = BKG;
                } else {
                    out = prm[MSK].value;
                }

            } else {

                /**** [ 1. - Analize frame ] ****/

                // FIXME Blur and threshold
                blur(mat[MASK], mat[BLUR], Size(set.var[BLUR_VL], set.var[BLUR_VL]));
                threshold(mat[BLUR], mat[BLOB], set.var[THR_VL], 255, THRESH_BINARY);

                // FIXME Matrix with new blobs
                bitwise_not(mat[PRV], mat[PRV]);
                bitwise_and(mat[PRV], mat[BLOB], mat[NEW]);
                mat[PRV] = mat[OLD].clone();
                bitwise_xor(mat[NEW], mat[BLOB], mat[OLD]);
                dilate(mat[BLOB], mat[PRV], kernel9, Point(-1, -1), 6);

                // FIXME Find new contours
                vector<vector<Point>> contours;
                findContours(mat[NEW], contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                contourSize = contours.size();

                // Definitions
                vector<RotatedRect> minRect(contourSize);
                vector<Point> pointA(contourSize);
                vector<Point> pointB(contourSize);
                vector<Point> pointM(contourSize);
                vector<int> len(contourSize);
                vector<int> dir(contourSize);


                if (contourSize > 0) {

                    /**** [ 1.1. - Approx segments ] ****/

                    for (int i = 0; i < contourSize; i++) {
                        minRect[i] = minAreaRect(Mat(contours[i]));
                        extremes(minRect[i], pointA[i], pointB[i]);
                        pointM[i] = (pointA[i] + pointB[i]) / 2;
                        len[i] = norm(pointB[i] - pointA[i]);
                        dir[i] = calc_dir(pointA[i], pointB[i]);
                    }


                    /**** [ 1.2. - Edit alive rays ] ****/

                    aliveSize = size(alive);
                    if (aliveSize > 0) {

                        for (int i = 0; i < aliveSize; i++) {

                            bool edit_ray = false;
                            id = alive[i];
                            dig_ray(s, id, ray);
                            Mat tmp = INIT_MAT;
                            Point rayM = med(ray.Ax, ray.Ay, ray.Bx, ray.By);
                            line(tmp, Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), Scalar(255), 2);


                            /**** [ 1.2.1. - Connect aligned blobs ] ****/

                            for (int j = 0; j < contourSize; j++) {

                                if (ray.len <= MIN_LEN && len[j] <= MIN_LEN) {

                                    if (norm(rayM - pointM[j]) < 2 * MIN_LEN) {
                                        line(tmp, pointA[j], pointB[j], Scalar(255), 2);
                                        line(tmp, rayM, pointM[j], Scalar(255), 2);
                                        drawContours(mat[NEW], contours, j, Scalar(0), FILLED);
                                        edit_ray = true;
                                    }

                                    /** Start edit **/
//                                } else if (abs(ray.frm - f) <= 5) {
//                                    line(tmp, pointA[j], pointB[j], Scalar(255), 2);
//                                    line(tmp, rayM, pointM[j], Scalar(255), 2);
//                                    drawContours(mat[NEW], contours, j, Scalar(0), FILLED);
//                                    edit_ray = true;
                                    /** End edit **/

                                } else {

                                    Mat tmpi = INIT_MAT;
                                    Mat tmpj = INIT_MAT;

                                    ellipse(tmpi, rayM,
                                            Size(max(set.var[LEN_MULT] * ray.len, MIN_LEN), MIN_LEN),
                                            ray.dir, 0, 360, Scalar(255), FILLED);
                                    ellipse(tmpj, Point(pointM[j]),
                                            Size(max(set.var[LEN_MULT] * len[j], MIN_LEN), MIN_LEN),
                                            dir[j], 0, 360, Scalar(255), FILLED);

                                    int area_i = countNonZero(tmpi);
                                    int area_j = countNonZero(tmpj);
                                    bitwise_and(tmpi, tmpj, tmpj);
                                    int area_and = countNonZero(tmpj);

                                    if ((double) area_and / double(min(area_i, area_j)) * 100 > set.var[AREA_PERC]) {

                                        tmpj *= 0;
                                        line(tmpj, Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), Scalar(255), 2);
                                        line(tmpj, pointA[j], pointB[j], Scalar(255), 2);
                                        line(tmpj, rayM, pointM[j], Scalar(255), 2);

                                        vector<vector<Point>> contour(1);
                                        findContours(tmpj, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                        RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                                        Point A, B;
                                        extremes(approxRect, A, B);

                                        int dir_diff = (int) abs(ray.dir - calc_dir(A, B));
                                        int len_diff = (int) abs((ray.len + len[j]) - norm(A - B));
                                        int dist_diff = calc_dist(Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), (A + B) / 2);

                                        if (dir_diff < set.var[DIR_DIFF] && dist_diff < MIN_LEN && len_diff < 4 * MIN_LEN) {
                                            line(tmp, pointA[j], pointB[j], Scalar(255), 2);
                                            line(tmp, rayM, pointM[j], Scalar(255), 2);
                                            drawContours(mat[NEW], contours, j, Scalar(0), FILLED);
                                            edit_ray = true;
                                        }

                                    }

                                }

                            }


                            /**** [ 1.2.2. - Update rays and contours ] ****/

                            if (edit_ray) {

                                vector<vector<Point>> contour(1);
                                findContours(tmp, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                                Point A, B;
                                extremes(approxRect, A, B);
                                ray = {A.x, A.y, B.x, B.y, int(norm(B - A)), int(calc_dir(A, B)), ray.frm};
                                edit(s, id, ray);

                                findContours(mat[NEW], contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                contourSize = contours.size();
                                minRect.resize(contourSize);
                                pointA.resize(contourSize);
                                pointB.resize(contourSize);
                                pointM.resize(contourSize);
                                len.resize(contourSize);
                                dir.resize(contourSize);

                                // Update segments
                                if (contourSize > 0) {
                                    for (int j = 0; j < contourSize; j++) {
                                        minRect[j] = minAreaRect(Mat(contours[j]));
                                        extremes(minRect[j], pointA[j], pointB[j]);
                                        pointM[j] = (pointA[j] + pointB[j]) / 2;
                                        len[j] = norm(pointB[j] - pointA[j]);
                                        dir[j] = calc_dir(pointA[j], pointB[j]);
                                    }
                                }

                            }

                        }

                    }

                }


                /**** [ 2. - Connect aligned rays ] ****/

                for (int i = 0; i < aliveSize; i++) {

                    bool edit_ray = false;
                    id = alive[i];
                    if (id != 0) {

                        dig_ray(s, id, ray);
                        Mat tmp = INIT_MAT;
                        Point rayM = med(ray.Ax, ray.Ay, ray.Bx, ray.By);
                        line(tmp, Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), Scalar(255), 2);

                        for (int j = 0; j < aliveSize; j++) {

                            int id0 = alive[j];
                            if (id != 0) {

                                int count, count0;
                                track ray0;
                                dig_ray(s, id0, ray0);
                                dig_count(s, id, count);
                                dig_count(s, id0, count0);

                                if (id0 != id && count0 == 0 && ray.len >= ray0.len) {

                                    Point ray0M = med(ray0.Ax, ray0.Ay, ray0.Bx, ray0.By);

                                    if (ray.len <= MIN_LEN && ray0.len <= MIN_LEN) {

                                        if (norm(rayM - ray0M) < 2 * MIN_LEN) {
                                            line(tmp, Point(ray0.Ax, ray0.Ay), Point(ray0.Bx, ray0.By), Scalar(255), 2);
                                            line(tmp, rayM, ray0M, Scalar(255), 2);
                                            pop(s, id0);
                                            alive[j] = 0;
                                            edit_ray = true;
                                        }

                                        /** Start edit **/
//                                    } else if (abs(ray.frm - ray0.frm) <= 5) {
//                                        line(tmp, Point(ray0.Ax, ray0.Ay), Point(ray0.Bx, ray0.By), Scalar(255), 2);
//                                        line(tmp, rayM, ray0M, Scalar(255), 2);
//                                        pop(s, id0);
//                                        alive[j] = 0;
//                                        edit_ray = true;
                                        /** End edit **/

                                    } else {

                                        Mat tmpi = INIT_MAT;
                                        Mat tmpj = INIT_MAT;

                                        ellipse(tmpi, rayM, Size(max(set.var[LEN_MULT] * ray.len, MIN_LEN), MIN_LEN),
                                                ray.dir, 0, 360, Scalar(255), FILLED);
                                        ellipse(tmpj, ray0M, Size(max(set.var[LEN_MULT] * ray0.len, MIN_LEN), MIN_LEN),
                                                ray0.dir, 0, 360, Scalar(255), FILLED);

                                        int area_i = countNonZero(tmpi);
                                        int area_j = countNonZero(tmpj);
                                        bitwise_and(tmpi, tmpj, tmpj);
                                        int area_and = countNonZero(tmpj);

                                        if ((double) area_and / double(min(area_i, area_j)) * 100 > set.var[AREA_PERC]) {

                                            tmpj *= 0;
                                            line(tmpj, Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), Scalar(255), 2);
                                            line(tmpj, Point(ray0.Ax, ray0.Ay), Point(ray0.Bx, ray0.By), Scalar(255), 2);
                                            line(tmpj, rayM, ray0M, Scalar(255), 2);

                                            vector<vector<Point>> contour(1);
                                            findContours(tmpj, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                            RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                                            Point A, B;
                                            extremes(approxRect, A, B);

                                            int dir_diff = (int) abs(ray.dir - calc_dir(A, B));
                                            int len_diff = (int) abs(norm(A - B) - (ray.len + ray0.len)) * 2;
                                            int dist_diff = calc_dist(Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), (A + B) / 2);

                                            if (dir_diff < set.var[DIR_DIFF] && dist_diff < MIN_LEN && len_diff < norm(A - B)) {
                                                line(tmp, Point(ray0.Ax, ray0.Ay), Point(ray0.Bx, ray0.By), Scalar(255), 2);
                                                line(tmp, rayM, ray0M, Scalar(255), 2);
                                                pop(s, id0);
                                                alive[j] = 0;
                                                edit_ray = true;
                                            }

                                        }

                                    }

                                }

                            }

                        }

                        // Update ray and contours
                        if (edit_ray) {
                            vector<vector<Point>> contour(1);
                            findContours(tmp, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                            RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                            Point A, B;
                            extremes(approxRect, A, B);
                            ray = {A.x, A.y, B.x, B.y, int(norm(B - A)), int(calc_dir(A, B)), ray.frm};
                            edit(s, id, ray);
                        }

                    }

                }

                // Rearrange alive array
                for (int i = 0; i < aliveSize; i++) {
                    if (alive[i] == 0) {
                        for (int j = i; j < aliveSize; j++)
                            alive[j] = alive[j + 1];
                        i--;
                    }
                }


                /**** [ 3. - Populate ] ****/

                if (contourSize > 0) {

                    // Find disjointed tracks
                    if (contourSize == 1) {

                        ray = {pointA[0].x, pointA[0].y, pointB[0].x, pointB[0].y, len[0], dir[0], f};
                        push(s, ray);
                        top_id(s, id);
                        aliveNew[0] = id;
                        aliveNew[1] = -1;

                    } else {

                        // FIXME delete joint and srt
                        joint = new int *[contourSize];
                        for (int i = 0; i < contourSize; i++)
                            joint[i] = new int[contourSize + 1];

                        srt = new int[contourSize];
                        sort(srt, contourSize, len);

                        // Join segments (from longest to shortest)
                        int k;
                        for (int i = 0; i < contourSize; i++) {

                            k = 0;
                            Mat tmpi = INIT_MAT;
                            ellipse(tmpi, pointM[srt[i]],
                                    Size(max(set.var[LEN_MULT] * len[srt[i]], MIN_LEN), MIN_LEN),
                                    dir[srt[i]], 0, 360, Scalar(255), FILLED);

                            for (int j = i; j < contourSize; j++) {

                                if (srt[i] != srt[j]) {

                                    if (len[srt[i]] <= MIN_LEN && len[srt[j]] <= MIN_LEN) {

                                        if (norm(pointM[srt[i]] - pointM[srt[j]]) < 2 * MIN_LEN)
                                            joint[srt[i]][k++] = srt[j];

                                    } else {

                                        Mat tmpj = INIT_MAT;
                                        ellipse(tmpj, pointM[srt[j]], Size(max(set.var[LEN_MULT] * len[srt[j]], MIN_LEN), MIN_LEN),
                                                dir[srt[j]], 0, 360, Scalar(255), FILLED);

                                        int area_j = countNonZero(tmpj);
                                        bitwise_and(tmpi, tmpj, tmpj);
                                        int area_and = countNonZero(tmpj);
                                        if ((double) area_and / (double) area_j * 100 > set.var[AREA_PERC])
                                            joint[srt[i]][k++] = srt[j];

                                    }

                                } else {

                                    joint[srt[i]][k++] = srt[j];

                                }
                            }

                            joint[srt[i]][k] = -1;

                        }

                        if (DEBUG)
                            debug(contourSize, joint, srt);

                        if (size(joint[srt[0]]) == contourSize) {
                            for (int i = 1; i < contourSize; i++)
                                joint[srt[i]][0] = -1;
                        } else if (contourSize > 2) {
                            disjoin(joint, srt, contourSize);
                        }

                        int idx = 0;
                        for (int i = 0; i < contourSize; i++) {

                            if (joint[i][0] >= 0) {

                                Mat tmp = INIT_MAT;
                                for (int j = 0; j < size(joint[i]); j++) {
                                    line(tmp, pointA[joint[i][j]], pointB[joint[i][j]], Scalar(255), 2);
                                    if (joint[i][j + 1] >= 0)
                                        line(tmp, pointM[joint[i][j]], pointM[joint[i][j + 1]], Scalar(255), 2);
                                }

                                vector<vector<Point>> contour(1);
                                findContours(tmp, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                                Point A, B;
                                extremes(approxRect, A, B);

                                ray = {A.x, A.y, B.x, B.y, int(norm(B - A)), int(calc_dir(A, B)), f};
                                push(s, ray);
                                top_id(s, id);
                                aliveNew[idx++] = id;
                                aliveNew[idx] = -1;

                            }

                        }

                    }

                }


                /**** [ 4. - Life or death ] ****/

                aliveSize = size(alive);
                if (aliveSize > 0) {

                    for (int i = 0; i < aliveSize; i++) {

                        int life = 1;
                        int count = 0;
                        id = alive[i];
                        dig_ray(s, id, ray);

                        Mat tmp = INIT_MAT;
                        line(tmp, Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), Scalar(255), 2);
                        dilate(tmp, tmp, kernel9, Point(-1, -1), 6);
                        bitwise_and(tmp, mat[OLD], tmp);

                        if (countNonZero(tmp) > 0) {
                            edit(s, id, true);
                        } else {
                            dig_life(s, id, life);
                            if (life >= set.var[MIN_LIFE])
                                edit(s, id, false);
                            else
                                pop(s, id);
                            alive[i] = 0;
                        }

                        dig_life(s, id, life);
                        dig_count(s, id, count);

                        if (life == set.var[MIN_LIFE] && count == 0)
                            edit(s, id, ++counter);

                    }

                    // Rearrange alive array
                    for (int i = 0; i < aliveSize; i++) {
                        if (alive[i] == 0) {
                            for (int j = i; j < aliveSize; j++)
                                alive[j] = alive[j + 1];
                            i--;
                        }
                    }

                }


                /**** [ 5. - Show the magic ] ****/

                // Background transparency
                if (prm[BAK].value > 0) {
                    cvtColor(mat[prm[MSK].value], mat[RGB], COLOR_GRAY2BGR);
                    mat[BKG] = crop * prm[BAK].value / 10 + mat[RGB] * (10 - prm[BAK].value) / 10;
                    out = BKG;
                } else {
                    out = prm[MSK].value;
                }

                // Draw rays
                if (prm[RAY].value || prm[NUM].value) {
                    for (int i = 0; i < aliveSize; i++) {
                        id = alive[i];
                        int count;
                        dig_count(s, id, count);
                        if (count > 0) {
                            dig_ray(s, id, ray);
                            if (prm[RAY].value)
                                line(mat[out], Point(ray.Ax, ray.Ay), Point(ray.Bx, ray.By), Scalar((prm[BAK].value > 0 ? 20 : 255), 120, 240), 1);
                            if (prm[NUM].value) {
                                sprintf(bfr, "%i", count);
                                putText(mat[out], bfr, med(ray.Ax, ray.Ay, ray.Bx, ray.By), FONT_HERSHEY_DUPLEX, 0.8, Scalar(240, 240, 240), 1);
                            }
                        }
                    }
                }

                // Draw contours
                if (prm[CON].value) {
                    for (int i = 0; i < contourSize; i++)
                        drawContours(mat[out], contours, i, Scalar(0, 255, 0));
                }

                // Draw segments
                if (prm[SEG].value) {
                    for (int i = 0; i < contourSize; i++) {
                        line(mat[out], pointA[i], pointB[i], Scalar(0, 240, 255), 2);
                        if (contourSize > 1) {
                            for (int j = 0; joint[i][j] != -1; j++) {
                                if (joint[i][j + 1] != -1)
                                    line(mat[out], pointM[joint[i][j]], pointM[joint[i][j + 1]], Scalar(200, 0, 200), 1);
                            }
                        }
                    }
                }

                // Draw ellipses
                if (prm[ELP].value) {
                    for (int i = 0; i < contourSize; i++)
                        ellipse(mat[out], pointM[i], Size(max(set.var[LEN_MULT] * len[i], MIN_LEN), MIN_LEN), dir[i], 0, 360, Scalar(255, 10, 10), 1);
                }


                /**** [ 6. - Concat aliveNew ] ****/

                aliveSize = size(alive);
                for (int i = 0; i <= size(aliveNew); i++)
                    alive[aliveSize + i] = aliveNew[i];
                aliveNew[0] = -1;

            }




/**** [ Output ] ****/

            // Fps
            bool del = false;
            tm.stop();
            double tmp = tm.getTimeSec();
            time = cap.get(CAP_PROP_POS_MSEC) / 1000;
            if (intlen(fps) > intlen(1 / tmp))
                del = true;
            fps = int(1 / tmp);
            tm.reset();
            tm.start();


            // User interface
            if (GUI) {
                for (int i = 0; i < SET_PRM_NUM_USER; i++) {
                    gotoxy(2, 4 + 2 * i);
                    if (select == i)
                        cout << ln_s << "╴";
                    else
                        cout << ln_v << " ";
                    if (prm[i].txt)
                        coutxy(prm[i].str[prm[i].value], 14, 4 + 2 * i);
                    else
                        coutbar(prm[i].value, 0, 10, 14, 4 + 2 * i);
                }
                cout << f_wht;
                coutxy(counter, 22, 15, RIGHT);
                cout << c_normal;
                if (del) {
                    coutxy(" ", 21 - intlen(fps), 17);
                }
                coutxy(fps, 22, 17, RIGHT);
                if (set.SEC_FRAME == 0) {
                    if (SOURCE == VID) {
                        char bfr_[16];
                        sprintf(bfr, "%i", (int) time);
                        strcat(bfr, "/");
                        sprintf(bfr_, "%i", totTime);
                        strcat(bfr, bfr_);
                        coutxy(bfr, 22, 19, RIGHT);
                    } else {
                        coutxy((int) time, 22, 19, RIGHT);
                    }
                } else {
                    coutxy((int) f, 22, 19, RIGHT);
                }
                cout << c_normal;
                print(s, 29, 4, 17);
            }

            // Video output
            imshow("Tracking", mat[out]);
            if (prm[MSK].value == BLNK)
                mat[BLNK] *= 0;
            videoSource.write(frame);
            videoCrop.write(mat[out]);
        }
    }




/**** [ Restore ] ****/

    // Release output
    destroyAllWindows();
    cap.release();
    videoSource.release();
    videoCrop.release();
    save(set.savePath, videoSourceName, videoCropName);
    save(s, set.savePath);
    deinit(s);
    delete[] joint;

    // Reset console
    cerr.rdbuf(cerrSbuf);
    cerr << c_normal;
    cout << c_normal;
    cursor();
    cooked();
    echo();

    return 0;

}
