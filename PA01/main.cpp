#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "GaussGen/boxmuller.c"
#include <math.h>
#include "headers/MatrixMath.h"
#include "headers/Bound.h"

#define pi 3.14159265359

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

    image.at<Vec3b>(Point(100+x,y))[colorCode] = (int) (1 * scale);

    if(image.at<Vec3b>(Point(100+x,y))[colorCode] > 255)
        image.at<Vec3b>(Point(100+x,y))[colorCode] = 255;
}

double calcProbability(double point[2], double mean[2], double StDev[2])
{
    double nx = (point[0] - mean[0]) / StDev[0];//n
    double ny = (point[1] - mean[1]) / StDev[1];//n

    double PrX = (1/(2*pi)) * exp(-nx*nx / 2); //P(X | Class) = P(N = n)
    double PrY = (1/(2*pi)) * exp(-ny*ny / 2); //P(Y | Class)

    double PrXYgC = PrX * PrY;//P(X,Y | Class)

    return PrXYgC;
}

int main(int argc, char** argv)
{
    int seed = time(NULL);
    const int numPoints = 10000;
    double pointSet[numPoints*2][3];//{x,y,class}
    double probabilityA = 0.5;
    double probabilityB = 0.5;

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
    double meanv[2][1] = {{1.0},{1.0}};//vertical 2d matrix
    double covarianceA[2][2] = {{1.0f,0.0f},{0.0f,1.0f}};
    double stdDev[2][2] = {{sqrt(1.0f),0.0f},{0.0f,sqrt(1.0f)}};
    double stdDev2[2] = {sqrt(1.0f),sqrt(1.0f)};
    double meanb[2] = {4.0f,4.0f};
    double meanbv[2][1] = {{4.0},{4.0}};//vertical 2d matrix
    double covarianceB[2][2] = {{1.0f,0.0f},{0.0f,1.0f}};
    double stdDevb[2][2] = {{sqrt(1.0f),0.0f},{0.0f,sqrt(1.0f)}};
    double stdDevb2[2] = {sqrt(1.0f),sqrt(1.0f)};
	double cBound[100][2];

    int xOffset = 100;
    int yOffset = -100;

    double spread = 10.0;

    for(int i=0; i< 10000; i++)
    {
        pointSet[i][0] = 0.0f;
        pointSet[i][1] = 0.0f;
        pointSet[i][2] = 0.0f;//class

        box_muller2d(pointSet[i], mean, stdDev);

        //plot the points

        int x = (pointSet[i][0] * spread) + xOffset;
        int y = 500 - (pointSet[i][1] * spread) + yOffset;

        drawPixel(image, Magenta, y, x, 255, 1.0f);

        //cout<<"("<<pointSet[i][0]<<","<<pointSet[i][1]<<")"<<endl;
    }

    for(int i=0; i< 10000; i++)
    {
        pointSet[i+numPoints][0] = 0.0f;
        pointSet[i+numPoints][1] = 0.0f;
        pointSet[i+numPoints][2] = 1.0f;//class

        box_muller2d(pointSet[i+numPoints], meanb, stdDevb);

        //plot the points

        int x = (pointSet[i+numPoints][0] * spread) + xOffset;
        int y = 500 - (pointSet[i+numPoints][1] * spread) + yOffset;

        drawPixel(image, Cyan, y, x, 255, 1.0f);

        //cout<<"("<<pointSet[i+numPoints][0]<<","<<pointSet[i+numPoints][1]<<")"<<endl;
    }


    int correct = 0;
    int incorrect = 0;

    //run through our data again with the classifier
    for(int i=0; i<20000; i++)
    {
        double pAc = calcProbability(pointSet[i], mean, stdDev2) * probabilityA;
        double pBc = calcProbability(pointSet[i], meanb, stdDevb2) * probabilityB;

        if(pAc > pBc)
        {
            if(pointSet[i][2] == 1.0f)
                incorrect ++;
            else
                correct ++;
        }
        else
        {
            if(pointSet[i][2] == 1.0f)
                correct ++;
            else
                incorrect ++;
        }
    }

    cout<<"Correct classified: "<<correct<<endl;
    cout<<"Incorrect classified: "<<incorrect<<endl;




	// Access boundary points through cBound
	makeChernoff(cBound,meanv,meanbv,covarianceA,covarianceB);
    double chernoffScale = 300.0;

    //plot it
    for(int i=0; i<10000; i++)
    {
        int x = (cBound[i][0] * chernoffScale) + 50;
        int y = 500 - (cBound[i][1] * chernoffScale) + 0;

        drawPixel(image, Yellow, x, y, 255, 1.0f);
        drawPixel(image, Magenta, x, y, 255, 1.0f);
    }

    double minimumBeta = 0.0;
    double minimumY = 0.0;

    findMinB(cBound, minimumBeta, minimumY);

	cout << "The Beta that minimizes error for the Chernoff bound is: " << minimumBeta <<endl
		 << "Where e^-k(b) is : " << minimumY   << endl
		  <<"P(error) is equal to: "<< getProb(probabilityA,probabilityB,minimumY)<< endl<< endl;

	// calculating battacharyya bound
	double kb = getBattacharayyaBound(meanv,meanbv,covarianceA,covarianceB);

	cout << "The Battacharyya bound gives us k(0.5) = " << kb << endl
	   	<< "where e^-k(b) is: " << exp(-kb) << endl
		<<"P(error) is equal to: " << getProb(probabilityA,probabilityB,kb) << endl;




    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);
}
