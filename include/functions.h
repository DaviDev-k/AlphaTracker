#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "settings.h"

using namespace std;
using namespace cv;

bool findQuad(Mat &src, Mat &transmtx, int OUT_RES);

double calc_m(const Point &A, const Point &B);

int calc_dir(const Point &A, const Point &B);

int calc_dist(const Point &A, const Point &B, const Point &M);

void extremes(const RotatedRect &, Point &, Point &);

Point med(int Ax, int Ay, int Bx, int By);

int size(const int *);

void sort(int *, int, vector<int>);

/*void connect (int**, int*, int, vector <int>, vector <int>, vector <Point>, int MIN_LEN, int OUT_RES);*/
void disjoin(int **, const int *, int);

void debug(int contourSize, int **joint, const int *srt);

/*void save (char path[]);*/
void save(char path[], char file1[], char file2[]);

void quit(int n);

#endif // FUNCTIONS_H
