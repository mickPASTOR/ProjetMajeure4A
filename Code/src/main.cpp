
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "scene.hpp"

int main(int, char**)
{
    scene scene_camera;

    cv::VideoCapture cap(0); // open the default camera
    // (for a second camera: index 1, etc..
    scene_camera.load_camera(cap);//Ouverture de la camera

    while(true)
    {
        scene_camera.draw_scene(cap);

        if(cv::waitKey(30) >= 0)      //Escape enables to quit the program
            break;
    }

    return 0;
}


