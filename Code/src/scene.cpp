#include "scene.hpp"
#include "hand_detection.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

scene::scene()
    :H_MIN_BLUE(27),
      H_MAX_BLUE(98),
      S_MIN_BLUE(46),
      S_MAX_BLUE(185),
      V_MIN_BLUE(62),
      V_MAX_BLUE(190),
      Sdilate_BLUE(6),
      Serode_BLUE(5),
      H_MIN_RED(145),
      H_MAX_RED(181),
      S_MIN_RED(117),
      S_MAX_RED(227),
      V_MIN_RED(118),
      V_MAX_RED(209),
      Sdilate_RED(6),
      Serode_RED(5),
      nbr_pts_min_contour(5000),
      rayon(10),
      init(true),
      drawing_color(cv::Vec3b(240,150,0)),
      etat(blue)
{}

void scene::load_camera(cv::VideoCapture &cap)
{
    if(!cap.isOpened())      // check if we succeeded
    {
        std::cerr<<"Failed to open Camera"<<std::endl;
        exit(1);
    }
}

void scene::draw_scene(cv::VideoCapture &cap)
{
    cv::Mat frame;                              // an OpenCV Image class

    //STRUCTURE DETECTION DU BLEU
    cv::Mat blue_image;                         //open CV Image for stocking treated camera image
    std::vector<cv::Point> blue_largest_contour;//Vecteur enregistrant le contour conservé après traitement de l'image camera
    std::vector<cv::Point> blue_hull;
    std::vector<cv::Vec4i> blue_convexDefect;
    std::vector<cv::Point> blue_finger;

    //STRUCTURE DETECTION DU ROUGE
    cv::Mat red_image;                          //open CV Image for stocking treated camera image
    std::vector<cv::Point> red_largest_contour; //Vecteur enregistrant le contour conservé après traitement de l'image camera
    std::vector<cv::Point> red_hull;
    std::vector<cv::Vec4i> red_convexDefect;
    std::vector<cv::Point> red_finger;

    int nbr_red_finger;//On stocke le nombre de doigts rouge présent devant la camera

    std::vector<int> position;//Vecteur qui stocke la position des images affiché par dessus l'image de la caméra

    cap >> frame;               // read an image from the webcam
    cv::flip(frame,frame,1);    //set image in the good way

    cv::Mat dessin_rafraich = cv::Mat::ones(frame.rows,frame.cols,frame.type()); //matrice de dessin remise a zero a chaque rafraichissement, pour afin le pointeur de façon ephemere

    //Initialisation unique des filtres, de la matrice de dessin et de la palette de choix
    if(init==true)  //resize
    {
        init_dessin(cap,frame,blue_image,red_image);
        load_palette();
        init=false;
    }

    //Blue treatment
    {
        blue_filter();//On définit les HSV pour qu'il filtre la couleur bleue
        blue_image = Treat_Image(frame);//On traite l'image de la caméra pour ne garder que le bleue
        blue_largest_contour = get_largest_contours(blue_image,nbr_pts_min_contour);//Calcul du plus grand contour bleue présent
        blue_hull = get_hull(blue_largest_contour);//On obtient l'enveloppe correspondant à ce contour
        blue_convexDefect = get_convexDefect(blue_largest_contour);//Obtention des points de convexités
        blue_finger = fingers_detection(blue_largest_contour,blue_convexDefect,frame);//Obtention des points correspondant au bout des doigts bleues
    }

    //Red treatment : On effectue le traitement similaire à celui effectué pour la couleur bleue
    {
        red_filter();
        red_image = Treat_Image(frame);
        red_largest_contour = get_largest_contours(red_image,nbr_pts_min_contour);
        red_hull = get_hull(red_largest_contour);
        red_convexDefect = get_convexDefect(red_largest_contour);
        red_finger = fingers_detection(red_largest_contour,red_convexDefect,frame);
        nbr_red_finger = red_finger.size();
    }

    position = show_palette(frame);
    action_determination(frame,blue_finger,position, nbr_red_finger);//On décide de l'action à effectuer selon le
    //nombre de doigts rouge
    equal_dessin(dessin_rafraich);

    //Show elements
    {
        show_contours(blue_largest_contour,frame);
        show_contours(red_largest_contour,frame);

        show_hull(blue_hull,frame);
        show_hull(red_hull,frame);

        show_finger(blue_finger,frame);
        show_finger(red_finger,frame);

        show_finger(blue_finger,dessin_rafraich);
        show_current_state(frame);
    }

    imshow("Camera",frame);
    imshow("Dessin",dessin_rafraich);

    cv::moveWindow("Dessin",690,0);
}

cv::Mat scene::Treat_Image(const cv::Mat &im)
{
    cv::Mat skin;

    //cv::GaussianBlur(skin,skin,cv::Size(3,3),0.1,0,cv::BORDER_CONSTANT);
    cv::blur(skin, skin, cv::Size(1, 1), cv::Point(-1, -1), cv::BORDER_DEFAULT);

    //Image dans l'espace HSV
    {
        //first convert our RGB image to HSV
        cvtColor(im, skin, cv::COLOR_BGR2HSV);
        //filter the image in HSV color space
        inRange(skin, cv::Scalar(trackbarWindow.H_MIN, trackbarWindow.S_MIN, trackbarWindow.V_MIN), cv::Scalar(trackbarWindow.H_MAX, trackbarWindow.S_MAX, trackbarWindow.V_MAX), skin);
    }

    //ouverture sur l'image seuillee
    cv::erode(skin,skin,cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(trackbarWindow.Serode,trackbarWindow.Serode)));
    cv::dilate(skin,skin,cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(trackbarWindow.Sdilate,trackbarWindow.Sdilate)));

    //fermeture sur l'image seuillee
    cv::dilate(skin,skin,cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(trackbarWindow.Sdilate,trackbarWindow.Sdilate)));
    cv::erode(skin,skin,cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(trackbarWindow.Serode,trackbarWindow.Serode)));

    return skin;
}

void scene::blue_filter()
{
    //On met les valeurs de HSV pour filtrer selon HSV définit pour le bleu
    /** Definition for filtering */
    trackbarWindow.H_MIN = H_MIN_BLUE;
    trackbarWindow.H_MAX = H_MAX_BLUE;
    trackbarWindow.S_MIN = S_MIN_BLUE;
    trackbarWindow.S_MAX = S_MAX_BLUE;
    trackbarWindow.V_MIN = V_MIN_BLUE;
    trackbarWindow.V_MAX = V_MAX_BLUE;

    trackbarWindow.Sdilate = Sdilate_BLUE;
    trackbarWindow.Serode = Serode_BLUE;
}

void scene::red_filter()
{
    //On met les valeurs de HSV pour filtrer selon HSV définit pour le rouge
    /** Definition for filtering */
    trackbarWindow.H_MIN = H_MIN_RED;
    trackbarWindow.H_MAX = H_MAX_RED;
    trackbarWindow.S_MIN = S_MIN_RED;
    trackbarWindow.S_MAX = S_MAX_RED;
    trackbarWindow.V_MIN = V_MIN_RED;
    trackbarWindow.V_MAX = V_MAX_RED;

    trackbarWindow.Sdilate = Sdilate_RED;
    trackbarWindow.Serode = Serode_RED;
}

void scene::init_dessin(cv::VideoCapture &cap, cv::Mat &frame,cv::Mat &blue_image, cv::Mat &red_image)
{
    dessin=cv::Mat::ones(frame.rows,frame.cols,frame.type());

    for(int i=1;i<frame.rows;i++)
        for(int j=1;j<frame.cols;j++)
            dessin.at<cv::Vec3b>(i,j)=cv::Vec3b(255,255,255);//On initialise le dessin tout blanc au départ

    init_blue_filtering(cap,frame,blue_image);//Calibrage du filtrage bleu
    init_red_filtering(cap,frame,red_image);//Calibrage du filtrage rouge
}

void scene::init_blue_filtering(cv::VideoCapture &cap, cv::Mat &frame, cv::Mat &blue_image)
{
    //BLUE INIT

    blue_filter();
    trackbarWindow.trackbarWindowName="TrackBar Bleue";
    trackbarWindow.createTrackbars();

    while(cv::waitKey(30)<=0)//On attend que l'utiisateur appuie sur une touche pour signaler le fin du calibrage bleue
    {
        blue_image=Treat_Image(frame);//Filtrage bleue
        imshow("Calibrage Bleu",blue_image);//On affichage le résultat du filtrage bleue

        /** Definition for filtering */
        //On change les valeurs du filtrage bleue en fonction du choix de l'utilisateur
        H_MIN_BLUE = trackbarWindow.H_MIN;
        H_MAX_BLUE = trackbarWindow.H_MAX;
        S_MIN_BLUE = trackbarWindow.S_MIN;
        S_MAX_BLUE = trackbarWindow.S_MAX;
        V_MIN_BLUE = trackbarWindow.V_MIN;
        V_MAX_BLUE = trackbarWindow.V_MAX;
        Sdilate_BLUE = trackbarWindow.Sdilate;
        Serode_BLUE = trackbarWindow.Serode;
        cap >> frame;        // read an image from the webcam
        cv::flip(frame,frame,1);    //set image in the good way
    }
    cv::destroyWindow("Calibrage Bleu");
    cv::destroyWindow(trackbarWindow.trackbarWindowName);
}

void scene::init_red_filtering(cv::VideoCapture &cap, cv::Mat &frame, cv::Mat &red_image)
{
    //Calibrage de la même facon que le calibrage effectué pour la couleur bleue

    //RED INIT
    red_filter();

    trackbarWindow.trackbarWindowName="TrackBar Rouge";
    trackbarWindow.createTrackbars();
    while(cv::waitKey(30)<=0)
    {
        red_image=Treat_Image(frame);
        imshow("Calibrage Rouge",red_image);
        /** Definition for filtering */
        H_MIN_RED = trackbarWindow.H_MIN;
        H_MAX_RED = trackbarWindow.H_MAX;
        S_MIN_RED = trackbarWindow.S_MIN;
        S_MAX_RED = trackbarWindow.S_MAX;
        V_MIN_RED = trackbarWindow.V_MIN;
        V_MAX_RED = trackbarWindow.V_MAX;
        Sdilate_RED = trackbarWindow.Sdilate;
        Serode_RED = trackbarWindow.Serode;

        cap >> frame;        // read an image from the webcam
        cv::flip(frame,frame,1);    //set image in the good way
    }
    cv::destroyWindow("Calibrage Rouge");
    cv::destroyWindow(trackbarWindow.trackbarWindowName);
}

void scene::painting(cv::Mat &frame,const std::vector<cv::Point> &points)
{
    //On peint des cercles autour des points conservés (bout des doigts bleues), concrètement, on remplace les pixels dans la matrice dessin en respectant l'équation d'un cercle
    for(int x=-rayon;x<=rayon;x++)                                          //x varie entre -r et r
    {
        for(int y=-rayon;y<=rayon;y++)                                      //y varie entre -r et r
        {
            if(x*x+y*y<=rayon*rayon)                                        //On verifie qu'il s'agit d'un cercle
                for(unsigned int i=0; i<points.size();i++)
                {
                    if(points[i].x+x>=1 && points[i].x+x<=frame.cols)       //On vérifie que l'on est encore sur l'image selon x
                        if(points[i].y+y>=1 && points[i].y+y<=frame.rows)   //On vérifie que l'on est encore sur l'image selon y
                            dessin.at<cv::Vec3b>(points[i]+cv::Point(x,y)) = drawing_color;
                }
        }
    }
}

void scene::action_determination(cv::Mat &frame,const std::vector<cv::Point> &points,const std::vector<int> position, int nbr_finger)
{
    cv::Point doigt_haut;
    int hauteur = 480;
    for(cv::Point doigt : points)
        if(doigt.y<=hauteur)
        {
            hauteur = doigt.y;
            doigt_haut = doigt;
        }

    if(nbr_finger==2)       //S'il y a 2 doigts rouges, on est en mode sélection
        selection_mode(doigt_haut,position);
    else if(nbr_finger==3)  //S'il y a 3 doigts rouges, on valide la sélection
        validation(etat);
    else if(nbr_finger!=4)  //S'il y a 4 doigts rouges, on ne fait rien, le peintre peut bouger la main de peinture librement
        painting(frame, points);
}

std::vector<int> scene::show_palette(cv::Mat &frame)
{
    std::vector<int> position;
    int offset=0;
    auto it = palette.cbegin();

    //Afichage de l'image etat en bas de la camera
    show_image(frame,*it,420,350);
    it++;

    //Affichage de toutes les autres images de selection
    for(;it<palette.cend();it++)
    {
        show_image(frame,*it,0,offset);

        position.push_back(offset);
        position.push_back((*it).cols+offset);
        offset += (*it).cols+7;
    }
    return position;
}

void scene::selection_mode(cv::Point p, std::vector<int> pos)
{
    //On regarde dans quel zone de l'image est le point p, et on renvoie l'indice correspondant à la zone de l'image
    //(Enum zone_image)
    int num_image=0;
    for(unsigned int i=0; i<pos.size()-1; i+=2)
    {
        if(p.y<50 && p.x>pos[i] && p.x<pos[i+1])
        {
            etat = static_cast<zone_image>(num_image);
            return;
        }
        num_image++;
    }
}

void scene::validation(zone_image ma_zone)
{
    //Lors d'une validation on effectue concrètement l'action séléctionné par l'utilisateur
    switch (ma_zone){
    case neutral:
        drawing_color = cv::Vec3b(96,96,96);//Blue,Green, Red
        break;
    case blue:
        drawing_color = cv::Vec3b(192,140,36);
        break;
    case green:
        drawing_color = cv::Vec3b(111,184,38);
        break;
    case violet:
        drawing_color = cv::Vec3b(133,67,178);
        break;
    case orange:
        drawing_color = cv::Vec3b(0,75,225);
        break;
    case red:
        drawing_color = cv::Vec3b(21,19,192);
        break;
    case gomme:
        drawing_color = cv::Vec3b(255,255,255);
        break;
    case save:
        cv::imwrite("../image/Mon_dessin.jpg",dessin);
        break;
    case reset:
        for(int i=1;i<dessin.rows;i++)
            for(int j=1;j<dessin.cols;j++)
                dessin.at<cv::Vec3b>(i,j) = cv::Vec3b(255,255,255);
        break;
    default:
        break;
    }
}

void scene::show_current_state(cv::Mat &frame)
{
    show_image(frame,palette[etat+1],420,550);
}

void scene::load_palette()
{
    palette.push_back(cv::imread("../data/etat.png"));
    palette.push_back(cv::imread("../data/neutre.png"));
    palette.push_back(cv::imread("../data/blue.png"));
    palette.push_back(cv::imread("../data/green.png"));
    palette.push_back(cv::imread("../data/purple.png"));
    palette.push_back(cv::imread("../data/orange.png"));
    palette.push_back(cv::imread("../data/red.png"));
    palette.push_back(cv::imread("../data/gomme.png"));
    palette.push_back(cv::imread("../data/save.png"));
    palette.push_back(cv::imread("../data/clean.png"));
}

void scene::equal_dessin(cv::Mat &dessin_rafraich)
{
    for(int i=0; i<dessin.rows;i++)
        for(int j=0;j<dessin.cols;j++)
            dessin_rafraich.at<cv::Vec3b>(i,j) = dessin.at<cv::Vec3b>(i,j);
}

void show_image(cv::Mat &frame, const cv::Mat &p, int offset_i, int offset_j)
{
    for(int i=0; i<frame.rows;i++)
        for(int j=0;j<frame.cols;j++)
            if(i<p.rows && j<p.cols)
                if(p.at<cv::Vec3b>(i,j) != cv::Vec3b(0,0,0))
                    frame.at<cv::Vec3b>(i+offset_i,j+offset_j) = p.at<cv::Vec3b>(i,j);
}
