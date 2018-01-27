#include "hand_detection.hpp"

std::vector<cv::Point> get_largest_contours(cv::Mat &threshedimg, int nbr_pts_min)
{
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    int largest_area = 0;
    unsigned int largest_contour_index = 0;

    //On obtient les contours de l'image
    cv::findContours(threshedimg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    for (unsigned int i = 0; i< contours.size(); i++) // Boucle sur chaque contours obtenus sur l'image
    {
        double a = contourArea(contours[i], false);  //  On obtient l'air du contour
        if (a>largest_area)
        {
            largest_area = a;
            largest_contour_index = i;                //On enregistre l'index du plus grand contour
        }
    }

    std::vector<cv::Point> largest_contour;
    if (contours.size() > 0)
        if(cv::contourArea(contours[largest_contour_index])>nbr_pts_min)//On vérifie que le contour obtenu
            //vérifie un nombre de points minimum que l'on a défini
            largest_contour=contours[largest_contour_index];

    return largest_contour;
}

std::vector<cv::Point> get_hull(const std::vector<cv::Point> &largest_contour)
{
    std::vector<cv::Point> hull(largest_contour.size());
    cv::convexHull(cv::Mat(largest_contour),hull);//On obtient l'enveloppe à partir des points du contour
    return hull;
}

std::vector<cv::Vec4i> get_convexDefect(const std::vector<cv::Point> &largest_contour)
{
    std::vector<int> hullI(largest_contour.size());
    cv::convexHull(cv::Mat(largest_contour),hullI);//On obtient tout d'abord l'enveloppe à partir des points du contour
    std::vector<cv::Vec4i>defect(hullI.size());

    if (hullI.size()>3)
        cv::convexityDefects(largest_contour,hullI,defect);//Obtention des points de convexités à partir du contour

    return defect;
}

std::vector<cv::Point> fingers_detection(const std::vector<cv::Point> &contour,const std::vector<cv::Vec4i> &convexDefect,cv::Mat &original)
{
    size_t count = contour.size();
    std::vector<cv::Point> vect_pt;

    if( count <100 )
        return vect_pt;

    //Recherche de plusieurs doigts
    vect_pt=several_fingers_detection(contour,convexDefect,original);

    if(vect_pt.empty())//Si l'on a pas trouvé plusieurs doigts, on réalise la recherche d'un seul doigt
        vect_pt=unique_finger_detection(contour,convexDefect,original);

    return vect_pt;
}

std::vector<cv::Point> several_fingers_detection(const std::vector<cv::Point> &contour,const std::vector<cv::Vec4i> &convexDefect,cv::Mat &original)
{
    bool i=true;
    cv::Rect rectangle=rectangle_hull(contour, original);
    double L=std::max(rectangle.width,rectangle.height);//On calcule le max entre la largeur et la hauteur du rectangle entourant le contour de la main
    std::vector<cv::Point> vect_pt;

    for(auto convex : convexDefect)
    {
        int startidx=convex[0]; cv::Point ptStart( contour[startidx] );
        int endidx=convex[1]; cv::Point ptEnd( contour[endidx] );
        int faridx=convex[2]; cv::Point ptFar( contour[faridx] );
        double a=sqrt(pow(ptFar.x-ptEnd.x,2)+pow(ptFar.y-ptEnd.y,2));
        double b=sqrt(pow(ptFar.x-ptStart.x,2)+pow(ptFar.y-ptStart.y,2));

        //Si l'angle entre le point et ses deux voisins et inférieure à 80 ° et si la distance entre
        //ce point et ces deux voisins est supérieure à 0,2 x L, on considère qu'il s'agit d'un doigt
        if(get_angle(ptStart,ptFar,ptEnd)<80 && a>0.2*L && b>0.2*L)
        {
            vect_pt.push_back(ptEnd);
            if(i)
                vect_pt.push_back(ptStart);
            i=false;
        }
    }

    return vect_pt;
}

std::vector<cv::Point> unique_finger_detection(const std::vector<cv::Point> &contour,const std::vector<cv::Vec4i> &convexDefect,cv::Mat &original)
{
    cv::Rect rectangle=rectangle_hull(contour, original);
    double L=std::max(rectangle.width,rectangle.height);//On calcule le max entre la largeur et la hauteur du rectangle entourant le contour de la main
    bool aucun_resultat=true;//Booléen indiquant si l'on a déjà trouvé un point correspondant aux critères de recherche
    std::vector<cv::Point> vect_pt;

    for(auto convex : convexDefect)//On parcours l'ensemble des points de convexités obtenues
    {
        int startidx=convex[0]; cv::Point ptStart( contour[startidx] );
        int endidx=convex[1]; cv::Point ptEnd( contour[endidx] );
        int faridx=convex[2]; cv::Point ptFar( contour[faridx] );
        double a=sqrt(pow(ptFar.x-ptEnd.x,2)+pow(ptFar.y-ptEnd.y,2));
        double b=sqrt(pow(ptFar.x-ptStart.x,2)+pow(ptFar.y-ptStart.y,2));

        //Si l'angle entre le point et ses deux voisins et inférieure à 150 ° et si la distance entre
        //ce point et ces deux voisins est supérieure à 0,2 x L, on considère qu'il s'agit d'un doigt
        if(get_angle(ptStart,ptFar,ptEnd)<150 && a>0.2*L && b>0.2*L && aucun_resultat)
        {
            vect_pt.push_back(ptEnd);
            aucun_resultat=false;//Si on a obtenu un résultat on arrête la recherche
        }
    }

    return vect_pt;
}

void show_contours(const std::vector<cv::Point> &contour,cv::Mat &original)
{
    if (contour.size() > 0)//Si le contour existe, on l'affiche
        cv::drawContours(original, std::vector<std::vector<cv::Point>> (1,contour),0, CV_RGB(0, 0, 255), 2, 8);
}

void show_hull(const std::vector<cv::Point> &hull,cv::Mat &original)
{
    if (hull.size() > 0)//Si l'enveloppe existe, on l'affiche
        cv::drawContours(original, std::vector<std::vector<cv::Point>>(1,hull), -1, CV_RGB(255, 0, 0), 2, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );
}

void show_finger(const std::vector<cv::Point> &vect_pt,cv::Mat &original)
{
    for(auto bout_doigt : vect_pt)
        cv::circle(original,bout_doigt,4,cv::Scalar(255,0,0),2);
    //On affiche un cercle aux points que l'on a obtenu pour les doigts
}

double get_angle(const cv::Point &ptStart,const cv::Point &ptFar,const cv::Point &ptEnd)
{
    //Connaissant trois points ptFar, ptEnd et ptStart, on calcul l'angle entre ces points en utilisant le theorem d'Al Kashi
    double a=sqrt(pow(ptFar.x-ptEnd.x,2)+pow(ptFar.y-ptEnd.y,2));
    double b=sqrt(pow(ptFar.x-ptStart.x,2)+pow(ptFar.y-ptStart.y,2));
    double c=sqrt(pow(ptEnd.x-ptStart.x,2)+pow(ptEnd.y-ptStart.y,2));

    return acos((-pow(c,2)+pow(a,2)+pow(b,2))/(2*a*b))*180/M_PI;
}

cv::Rect rectangle_hull(const std::vector <cv::Point> &contour, cv::Mat &original)
{
    cv::Rect boundRect;
    if(contour.size()>0)//Si le contour existe alors on renvoie le rectangle correspondant
    {
        boundRect=cv::boundingRect(cv::Mat(contour));//On utilise la fonction OpenCV qui renvoie le rectangle selon un contour
        cv::rectangle(original,boundRect,cv::Scalar::all(0),2,8,0);
    }
    return boundRect;
}
