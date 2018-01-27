#pragma once

#ifndef SCENE_HPP
#define SCENE_HPP

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "trackbar.hpp"

enum zone_image {neutral, blue, green, violet, orange, red, gomme, save, reset}; //Enumeration des différentes
//zones de l'image

class scene
{
public:

    /** Constructor */
    scene();

    /** Opening camera */
    void load_camera(cv::VideoCapture &cap);

    /** \brief Method called at every frame */
    void draw_scene(cv::VideoCapture &cap);

private: //functions

    /** Blue definition of HSV Limits for filtering */
    void blue_filter();

    /** Red definition of HSV Limits for filtering */
    void red_filter();

    /** Initialisation de la matrice stockant le dessin de l'utilisateur */
    void init_dessin(cv::VideoCapture &cap, cv::Mat &frame,cv::Mat &blue_image, cv::Mat &red_image);

    /** Method to init blue filtering (calibration of detected colors) */
    void init_blue_filtering(cv::VideoCapture &cap, cv::Mat &frame, cv::Mat &blue_image);

    /** Method to init red filtering (calibration of detected colors) */
    void init_red_filtering(cv::VideoCapture &cap, cv::Mat &frame, cv::Mat &red_image);

    /** Charge les images utiles a la palette de choix */
    void load_palette();

    /** Treating image from camera */
    cv::Mat Treat_Image(const cv::Mat &im);

    /** Painting from contours detection and applying on camera image */
    void painting(cv::Mat &frame, const std::vector<cv::Point> &points);

    /** Separating cases by number of red fingers */
    void action_determination(cv::Mat &frame, const std::vector<cv::Point> &points,const std::vector<int> position, int nbr_finger);

    /** Placement des images en haut de la camera pour la selection et renvoi le positions des couleurs dans l'image */
    std::vector<int> show_palette(cv::Mat &frame);

    /** Action à réaliser selon la zone de l'écran touché */
    void validation(zone_image ma_zone);

    /** Mode de selection des couleurs / sauvegarde... */
    void selection_mode(cv::Point p, std::vector<int> pos);

    /** Affiche l'etat en selection */
    void show_current_state(cv::Mat &frame);

    /** Egalise la matrice attribut permetteant le dessin avec une matrice temporaire a chaque rafraichissement (permet l'affichage ephemere de la position des doigts */
    void equal_dessin(cv::Mat &dessin_rafraich);

private : //attributes

    /** Definition for filtering blue color */
    int H_MIN_BLUE;
    int H_MAX_BLUE;
    int S_MIN_BLUE;
    int S_MAX_BLUE;
    int V_MIN_BLUE;
    int V_MAX_BLUE;
    int Sdilate_BLUE;
    int Serode_BLUE;
    /** Definition for filtering red color */
    int H_MIN_RED;
    int H_MAX_RED;
    int S_MIN_RED;
    int S_MAX_RED;
    int V_MIN_RED;
    int V_MAX_RED;
    int Sdilate_RED;
    int Serode_RED;

    /** Toutes les trackbar qui permettent le calibrage des filtres sont manipulées à partir de cet attribut */
    trackbar trackbarWindow;

    /** Number of limits points for a contour to be conserved */
    int nbr_pts_min_contour;

    /** Definition of painter circle size */
    int rayon;

    /** Dessin réalisé enregistré dans une matrice (vive le francais :)) */
    cv::Mat dessin;

    /** Booléen pour l'initialisation de la matrice dessin */
    bool init;

    /** Couleur actuelle de peinture */
    cv::Vec3b drawing_color;

    /** Etat de la main bleue */
    zone_image etat;

    /** Palette de choix */
    std::vector<cv::Mat> palette;
};

void show_image(cv::Mat &frame, const cv::Mat &p, int offset_i=0, int offset_j=0);

#endif
