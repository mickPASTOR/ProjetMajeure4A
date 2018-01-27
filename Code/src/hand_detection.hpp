#pragma once

#ifndef HAND_HPP
#define HAND_HPP

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

/** Ensemble de fonctions permettant de gerer la detection des mains,
 *  et l'affichage des informations interressantes, comme les contours,
 *  et les points indiquant les doigts. */

/** Obtention du plus grand contour détecté sur l'image */
std::vector<cv::Point> get_largest_contours(cv::Mat &threshedimg, int nbr_pts_min);

/** Hull detections on image */
std::vector<cv::Point> get_hull(const std::vector<cv::Point> &largest_contour);

/** Convexity defects detections on hull */
std::vector<cv::Vec4i> get_convexDefect(const std::vector<cv::Point> &largest_contour);

/** Detect fingers on image */
std::vector<cv::Point> fingers_detection(const std::vector<cv::Point> &contour,const std::vector<cv::Vec4i> &convexDefect, cv::Mat &original);

/** Detect several fingers on image */
std::vector<cv::Point> several_fingers_detection(const std::vector<cv::Point> &contour,const std::vector<cv::Vec4i> &convexDefect,cv::Mat &original);

/** Detect unique finger on image */
std::vector<cv::Point> unique_finger_detection(const std::vector<cv::Point> &contour,const std::vector<cv::Vec4i> &convexDefect,cv::Mat &original);

/** Showing contours on image */
void show_contours(const std::vector<cv::Point> &contour, cv::Mat &original);

/** Showing hulls on image */
void show_hull(const std::vector<cv::Point> &hull, cv::Mat &original);

/** Showing fingers on image */
void show_finger(const std::vector<cv::Point> &vect_pt,cv::Mat &original);

/** Calcul l'angle entre 3 points */
double get_angle(const cv::Point &ptStart,const cv::Point &ptFar,const cv::Point &ptEnd);

/** Retourne le rectangle qui encadre le contour envoyé en paramètre */
cv::Rect rectangle_hull(const std::vector <cv::Point> &contour, cv::Mat &original);

#endif
