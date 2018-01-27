#pragma once

#ifndef TRACKBAR_HPP
#define MY_WINDOW_HPP

#include <string>

/** Declaration of the Tackbar class */
struct trackbar
{
    trackbar();

    /** Création du trackbar */
    void createTrackbars();

    /** Nom du trackbar */
    std::string trackbarWindowName;

    /** Definition of HSV Limits for filtering  and circle size for erosion and dilatation treatement*/
    int H_MIN,H_MAX,S_MIN,S_MAX,V_MIN,V_MAX,Sdilate,Serode;
};

/** Action a effectuer lors du glissement d'un trackbar */
void on_trackbar(int, void *);


#endif
