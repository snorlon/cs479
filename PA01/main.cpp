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

void drawPixel(Mat &image, const int colorCode, int x, int y, double amount)
{
    //stay within the bounds for rendering, do not render off screen
    if(x >= 500)
        x = 499;
    if(y >= 500)
        y = 499;
    if(x < 0)
        x = 0;
    if(y < 0)
        y = 0;

    //update the color corresponding to the code with an intensity dictated by "amount"
    image.at<Vec3b>(Point(100+x,y))[colorCode] = amount;
}

double calcProbability(double point[2], double mean[2][1], double StDev[2][2])
{
    double nx = (point[0] - mean[0][0]) / StDev[0][0];//n
    double ny = (point[1] - mean[1][0]) / StDev[1][1];//n

    double PrX = (1/(2*pi)) * exp(-nx*nx / 2); //P(X | Class) = P(N = n)
    double PrY = (1/(2*pi)) * exp(-ny*ny / 2); //P(Y | Class)

    double PrXYgC = PrX * PrY;//P(X,Y | Class)

    return PrXYgC;
}

int main(int argc, char** argv)
{
    //program variables
    int seed = time(NULL);
    const int numPoints = 10000;
    double pointSet[numPoints*2][3];//{x,y,class}
    double probabilityA = 0.5;
    double probabilityB = 0.5;
    double meanA[2][1] = {{1.0},{1.0}};
    double covarianceA[2][2] = {{1.0,0.0},{0.0,1.0}};
    double meanB[2][1] = {{4.0},{4.0}};
    double covarianceB[2][2] = {{1.0f,0.0f},{0.0f,1.0f}};
	double chernoffBoundPoints[10000][2];

    double stdDevA[2][2] = {{0.0,0.0},{0.0,0.0}};
    double stdDevB[2][2] = {{0.0,0.0},{0.0,0.0}};

    int xOffset = 100;//for rendering points
    int yOffset = -100;

    int correct = 0;
    int incorrect = 0;

    double spread = 10.0;//a multiplication factor for rendering points of the two classes


    double minimumBeta = 0.0;
    double minimumY = 0.0;
    double chernoffScale = 300.0;//used for rendering the chernoff bound
    double kb;//k(b)


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

    //mean for class A
    if(argc > 3)
    {
        //parameters 2 through 4 are for mean matrices
        meanA[0][0] = atof(argv[2]);
        meanA[1][0] = atof(argv[3]);
    }

    //mean for class B
    if(argc > 5)
    {
        //parameters 2 through 4 are for mean matrices
        meanB[0][0] = atof(argv[4]);
        meanB[1][0] = atof(argv[5]);
    }

    //class A covariance
    if(argc > 9)
    {
        //parameters 6 through 14 are for covariance matrices
        covarianceA[0][0] = atof(argv[6]);
        covarianceA[0][1] = atof(argv[7]);
        covarianceA[1][0] = atof(argv[8]);
        covarianceA[1][1] = atof(argv[9]);
    }

    //class B covariance
    if(argc > 13)
    {
        //parameters 6 through 14 are for covariance matrices
        covarianceB[0][0] = atof(argv[10]);
        covarianceB[0][1] = atof(argv[11]);
        covarianceB[1][0] = atof(argv[12]);
        covarianceB[1][1] = atof(argv[13]);
    }

    //prior probability for class A & class B
    if(argc > 15)
    {
        probabilityA = atof(argv[14]);
        probabilityB = atof(argv[15]);
    }

    //calculate standard deviation
    stdDevA[0][0] = sqrt(covarianceA[0][0]);//since in this case variance = covariance and sqrt(variance) = standard deviation
    stdDevA[1][1] = sqrt(covarianceA[1][1]);
    stdDevB[0][0] = sqrt(covarianceB[0][0]);
    stdDevB[1][1] = sqrt(covarianceB[1][1]);

    //initialize our RNG seed
    srand(seed);

    //generate points for class A
    for(int i=0; i< 10000; i++)
    {
        pointSet[i][0] = 0.0f;
        pointSet[i][1] = 0.0f;
        pointSet[i][2] = 0.0f;//class

        box_muller2d(pointSet[i], meanA, stdDevA);

        //plot the points

        int x = (pointSet[i][0] * spread) + xOffset;
        int y = 500 - (pointSet[i][1] * spread) + yOffset;

        drawPixel(image, Magenta, y, x, 1.0);

        //cout<<"("<<pointSet[i][0]<<","<<pointSet[i][1]<<")"<<endl;
    }

    //generate points for class B
    for(int i=0; i< 10000; i++)
    {
        pointSet[i+numPoints][0] = 0.0f;
        pointSet[i+numPoints][1] = 0.0f;
        pointSet[i+numPoints][2] = 1.0f;//class

        box_muller2d(pointSet[i+numPoints], meanB, stdDevB);

        //plot the points

        int x = (pointSet[i+numPoints][0] * spread) + xOffset;
        int y = 500 - (pointSet[i+numPoints][1] * spread) + yOffset;

        drawPixel(image, Cyan, y, x, 1.0);

        //cout<<"("<<pointSet[i+numPoints][0]<<","<<pointSet[i+numPoints][1]<<")"<<endl;
    }

    //run through our data again with the classifier, count if it gets calculated correctly or not
    for(int i=0; i<20000; i++)
    {
        double pAc = calcProbability(pointSet[i], meanA, stdDevA) * probabilityA;
        double pBc = calcProbability(pointSet[i], meanB, stdDevB) * probabilityB;

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


	// Access boundary points through chernoffBoundPoints, generate points for use
	makeChernoff(chernoffBoundPoints,meanA,meanB,covarianceA,covarianceB);

    //plot it
    for(int i=0; i<10000; i++)
    {
        int x = (chernoffBoundPoints[i][0] * chernoffScale) + 50;
        int y = 500 - (chernoffBoundPoints[i][1] * chernoffScale) + 0;

        drawPixel(image, Yellow, x, y, 0.75);
        drawPixel(image, Magenta, x, y, 0.75);
    }

    findMinB(chernoffBoundPoints, minimumBeta, minimumY);

	cout << "The Beta that minimizes error for the Chernoff bound is: " << minimumBeta <<endl
		 << "Where e^-k(b) is : " << minimumY   << endl
		  <<"P(error) is equal to: "<< getProb(probabilityA,probabilityB,minimumY)<< endl<< endl;

	// calculating battacharyya bound
	kb = getBattacharayyaBound(meanA,meanB,covarianceA,covarianceB);

	cout << "The Battacharyya bound gives us k(0.5) = " << kb << endl
	   	<< "where e^-k(b) is: " << exp(-kb) << endl
		<<"P(error) is equal to: " << getProb(probabilityA,probabilityB,exp(-kb)) << endl;


    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);
}
