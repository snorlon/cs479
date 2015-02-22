#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "GaussGen/boxmuller.c"

using namespace std;
using namespace cv;

const static int Yellow = 0;
const static int Magenta = 1;
const static int Cyan = 2;

void drawPixel(Mat &image, const int colorCode, int x, int y, int amount, double scale)
{
    if(x >= 500)
        x = 499;
    if(y >= 500)
        y = 499;
    if(x < 0)
        x = 0;
    if(y < 0)
        y = 0;

    image.at<Vec3b>(Point(100+x,y))[colorCode] += (int) (1 * scale);

    if(image.at<Vec3b>(Point(100+x,y))[colorCode] > 255)
        image.at<Vec3b>(Point(100+x,y))[colorCode] = 255;
}

int main(int argc, char** argv)
{
    int seed = time(NULL);
    const int numPoints = 10000;
    double pointSet[numPoints*2][3];//{x,y,class}
    int probabilityA = 0.5f;
    int probabilityB = 0.5f;

    double meanA[2] = {0.0f,0.0f};
    double meanB[2] = {0.0f,0.0f};

    double StDevA[2] = {0.0f,0.0f};
    double StDevB[2] = {0.0f,0.0f};



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

    double mean[2] = {1.0f,1.0f};
    double stdDev[2][2] = {{sqrt(1.0f),0.0f},{0.0f,sqrt(1.0f)}};

    int xOffset = -100;
    int yOffset = 100;

    int spread = 40;

    double total[2] = {0.0f,0.0f};

    for(int i=0; i< 10000; i++)
    {
        pointSet[i][0] = 0.0f;
        pointSet[i][1] = 0.0f;
        pointSet[i][2] = 0.0f;//class

        box_muller2d(pointSet[i], mean, stdDev);

        total[0] += pointSet[i][0];
        total[1] += pointSet[i][1];

        //plot the points

        int x = 500 - (pointSet[i][0] * spread) + xOffset;
        int y = pointSet[i][1] * spread + yOffset;

        drawPixel(image, Magenta, x, y, 1, 1.0f);

        //cout<<"("<<pointSet[i][0]<<","<<pointSet[i][1]<<")"<<endl;
    }

    //calculate the mean based on the total of all point values
    meanA[0] = total[0] / 10000;
    meanA[1] = total[1] / 10000;

    //reset totals
    total[0] = 0.0f;
    total[1] = 0.0f;

    //now math out standard deviation
    for(int i=0; i< 10000; i++)
    {
        //first calculate variation
        int difference[2] = {pointSet[i][0],pointSet[i][1]};

        difference[0] -= meanA[0];
        difference[1] -= meanA[1];

        total[0] += (difference[0]*difference[0]);
        total[1] += (difference[1]*difference[1]);
    }

    StDevA[0] = sqrt(total[0] / 10000);
    StDevA[1] = sqrt(total[1] / 10000);

    cout<<"Calculated mean of class A(0.0) is: ("<<meanA[0]<<","<<meanA[1]<<")"<<endl;
    cout<<"Calculated standard deviation of class A(0.0) is: ("<<StDevA[0]<<","<<StDevA[1]<<")"<<endl;

    double meanb[2] = {4.0f,4.0f};
    double stdDevb[2][2] = {{sqrt(1.0f),0.0f},{0.0f,sqrt(1.0f)}};

    //reset totals
    total[0] = 0.0f;
    total[1] = 0.0f;

    for(int i=0; i< 10000; i++)
    {
        pointSet[i+numPoints][0] = 0.0f;
        pointSet[i+numPoints][1] = 0.0f;
        pointSet[i+numPoints][2] = 1.0f;//class

        box_muller2d(pointSet[i+numPoints], meanb, stdDevb);

        total[0] += pointSet[i+numPoints][0];
        total[1] += pointSet[i+numPoints][1];

        //plot the points

        int x = 500 - (pointSet[i+numPoints][0] * spread) + xOffset;
        int y = pointSet[i+numPoints][1] * spread + yOffset;

        drawPixel(image, Cyan, x, y, 1, 1.0f);

        //cout<<"("<<pointSet[i+numPoints][0]<<","<<pointSet[i+numPoints][1]<<")"<<endl;
    }

    //calculate the mean based on the total of all point values
    meanB[0] = total[0] / 10000;
    meanB[1] = total[1] / 10000;

    //reset totals
    total[0] = 0.0f;
    total[1] = 0.0f;

    //now math out standard deviation
    for(int i=0; i< 10000; i++)
    {
        //first calculate variation
        int difference[2] = {pointSet[i+numPoints][0],pointSet[i+numPoints][1]};

        difference[0] -= meanB[0];
        difference[1] -= meanB[1];

        total[0] += (difference[0]*difference[0]);
        total[1] += (difference[1]*difference[1]);
    }

    StDevB[0] = sqrt(total[0] / 10000);
    StDevB[1] = sqrt(total[1] / 10000);


    cout<<"Calculated mean of class B(1.0) is: ("<<meanB[0]<<","<<meanB[1]<<")"<<endl;
    cout<<"Calculated standard deviation of class B(1.0) is: ("<<StDevB[0]<<","<<StDevB[1]<<")"<<endl;

    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);
}
