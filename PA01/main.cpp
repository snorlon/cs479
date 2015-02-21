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
        pointSet[i][0] = 0.0f;
        pointSet[i][1] = 0.0f;
        pointSet[i][2] = 0.0f;//class

        box_muller2d(pointSet[i], mean, stdDev);

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

        //cout<<"("<<pointSet[i+numPoints][0]<<","<<pointSet[i+numPoints][1]<<")"<<endl;
    }
}
