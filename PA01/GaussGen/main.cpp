#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "boxmuller.c"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    int seed = time(NULL);

    if(argc > 1)
    {
        //use the first parameter as a seed for replicating results
        seed = atoi(argv[1]);
    }

    srand(seed);

    float mean[2] = {1.0f,1.0f};
    float stdDev[2][2] = {{1.0f,0.0f},{0.0f,4.0f}};

    for(int i=0; i< 10000; i++)
    {
        float point[2] = {0.0f,0.0f};



        box_muller2d(point, mean, stdDev);

        cout<<"("<<point[0]<<","<<point[1]<<")"<<endl;
    }

    waitKey(0);
}
