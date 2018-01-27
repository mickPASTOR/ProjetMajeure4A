#include "trackbar.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

int *Sdil,*Sero;

trackbar::trackbar()
{}

void trackbar::createTrackbars()
{
    //create window for trackbars
    cv::namedWindow(trackbarWindowName,0);

    cv::createTrackbar("H_MIN", trackbarWindowName, &H_MIN, 255, on_trackbar);
    cv::createTrackbar("H_MAX", trackbarWindowName, &H_MAX, 255, on_trackbar);
    cv::createTrackbar("S_MIN", trackbarWindowName, &S_MIN, 255, on_trackbar);
    cv::createTrackbar("S_MAX", trackbarWindowName, &S_MAX, 255, on_trackbar);
    cv::createTrackbar("V_MIN", trackbarWindowName, &V_MIN, 255, on_trackbar);
    cv::createTrackbar("V_MAX", trackbarWindowName, &V_MAX, 255, on_trackbar);
    cv::createTrackbar("Sdilate", trackbarWindowName, &Sdilate, 10, on_trackbar);
    cv::createTrackbar("Serode", trackbarWindowName, &Serode, 10, on_trackbar);

    Sdil=&Sdilate;
    Sero=&Serode;
}

void on_trackbar(int, void*)
{
    //On gere ici les problèmes possibles duent à l'utilisateur (Sdilate et Serode égal à 0 impossible en réalité
    //donc on les fixes à 1 si tel est le cas.
    if(*Sdil == 0)
        *Sdil = 1;
    if(*Sero == 0)
        *Sero = 1;
}
