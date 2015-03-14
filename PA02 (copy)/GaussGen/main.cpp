#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "boxmuller.c"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    int seed = time(NULL);
    const int numPoints = 10000;
    float points[numPoints][2];

    if(argc > 1)
    {
        //use the first parameter as a seed for replicating results
        seed = atoi(argv[1]);
    }

    srand(seed);

    float mean[2] = {1.0f,1.0f};
    float stdDev[2][2] = {{1.0f,0.0f},{0.0f,1.0f}};

    for(int i=0; i< 10000; i++)
    {
        points[i][0] = 0.0f;
        points[i][1] = 0.0f;



        box_muller2d(points[i], mean, stdDev);

        cout<<"("<<points[i][0]<<","<<points[i][1]<<")"<<endl;
    }

    waitKey(0);
}
