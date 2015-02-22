#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "GaussGen/boxmuller.c"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    int seed = time(NULL);
    const int numPoints = 10000;
    float pointSet[numPoints*2][3];//{x,y,class}
    int probabilityA = 0.5f;
    int probabilityB = 0.5f;




    Mat image;

    image = imread( "graph.png", 1 );

    int width = image.cols;
    int height = image.rows;

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }




    if(argc > 1)
    {
        //use the first parameter as a seed for replicating results
        seed = atoi(argv[1]);
    }

    srand(seed);

    float mean[2] = {1.0f,1.0f};
    float stdDev[2][2] = {{1.0f,0.0f},{0.0f,1.0f}};

    int intensity = 1;
    int spread = 40;

    for(int i=0; i< 10000; i++)
    {
        pointSet[i][0] = 0.0f;
        pointSet[i][1] = 0.0f;
        pointSet[i][2] = 0.0f;//class

        box_muller2d(pointSet[i], mean, stdDev);

        //plot the points

        int x = 500 - (pointSet[i][0] * spread);
        int y = pointSet[i][1] * spread;

        if(x >= 500)
            x = 499;
        if(y >= 500)
            y = 499;
        if(x < 0)
            x = 0;
        if(y < 0)
            y = 0;

        Vec3b color = image.at<Vec3b>(Point(100+x,y));

        color[2] += intensity;

        if(color[2] > 255)
            color[2] = 255;

        image.at<Vec3b>(x,100+y) = color; 

        //cout<<"("<<pointSet[i][0]<<","<<pointSet[i][1]<<")"<<endl;
    }

    float meanb[2] = {4.0f,4.0f};
    float stdDevb[2][2] = {{1.0f,0.0f},{0.0f,1.0f}};

    for(int i=0; i< 10000; i++)
    {
        pointSet[i+numPoints][0] = 0.0f;
        pointSet[i+numPoints][1] = 0.0f;
        pointSet[i+numPoints][2] = 1.0f;//class

        box_muller2d(pointSet[i+numPoints], meanb, stdDevb);

        //plot the points

        int x = 500 - (pointSet[i+numPoints][0] * spread);
        int y = pointSet[i+numPoints][1] * spread;

        if(x >= 500)
            x = 499;
        if(y >= 500)
            y = 499;
        if(x < 0)
            x = 0;
        if(y < 0)
            y = 0;

        Vec3b color = image.at<Vec3b>(Point(100+x,y));

        color[1] += intensity;

        if(color[1] > 255)
            color[1] = 255;

        image.at<Vec3b>(x,100+y) = color; 

        //cout<<"("<<pointSet[i+numPoints][0]<<","<<pointSet[i+numPoints][1]<<")"<<endl;
    }


    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);
}
