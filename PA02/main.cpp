#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "GaussGen/boxmuller.c"
#include <math.h>
#include "headers/MatrixMath.h"
#include "headers/Bound.h"
#include <fstream>

#define pi 3.14159265359

using namespace std;
using namespace cv;

const static int Blue = 0;
const static int Green = 1;
const static int Red = 2;

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
    int numPoints = 10000;


    //give the option to take in number of points to generate
    if(argc > 17)
    {
        numPoints = atoi(argv[17]);
    }

    double pointSet[numPoints*2][3];//{x,y,class}
    double probabilityA = 0.5;
    double probabilityB = 0.5;
    double meanA[2][1] = {{1.0},{1.0}};
    double covarianceA[2][2] = {{1.0,0.0},{0.0,1.0}};
    double meanB[2][1] = {{4.0},{4.0}};
    double covarianceB[2][2] = {{1.0f,0.0f},{0.0f,1.0f}};
	double chernoffBoundPoints[numPoints*2][2];
    double outputGraph[numPoints][6];

    double stdDevA[2][2] = {{0.0,0.0},{0.0,0.0}};
    double stdDevB[2][2] = {{0.0,0.0},{0.0,0.0}};

    string filepathRoot = "output.txt";

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

    //give the option to take in a file name for saving to
    if(argc > 16)
    {
        string newPath(argv[16]);
        filepathRoot = newPath;
    }

    //calculate standard deviation
    stdDevA[0][0] = sqrt(covarianceA[0][0]);//since in this case variance = covariance and sqrt(variance) = standard deviation
    stdDevA[1][1] = sqrt(covarianceA[1][1]);
    stdDevB[0][0] = sqrt(covarianceB[0][0]);
    stdDevB[1][1] = sqrt(covarianceB[1][1]);

    //initialize our RNG seed
    srand(seed);

    //generate points for class A
    for(int i=0; i< numPoints; i++)
    {
        pointSet[i][0] = 0.0f;
        pointSet[i][1] = 0.0f;
        pointSet[i][2] = 0.0f;//class

        box_muller2d(pointSet[i], meanA, stdDevA);

        //plot the points

        int x = (pointSet[i][0] * spread) + xOffset;
        int y = 500 - (pointSet[i][1] * spread) + yOffset;

        drawPixel(image, Blue, y, x, 255);
        drawPixel(image, Green, y, x, 255);
        drawPixel(image, Red, y, x, 0);

        //cout<<"("<<pointSet[i][0]<<","<<pointSet[i][1]<<")"<<endl;
    }

    //generate points for class B
    for(int i=0; i< numPoints; i++)
    {
        pointSet[i+numPoints][0] = 0.0f;
        pointSet[i+numPoints][1] = 0.0f;
        pointSet[i+numPoints][2] = 1.0f;//class

        box_muller2d(pointSet[i+numPoints], meanB, stdDevB);

        //plot the points

        int x = (pointSet[i+numPoints][0] * spread) + xOffset;
        int y = 500 - (pointSet[i+numPoints][1] * spread) + yOffset;

        drawPixel(image, Blue, y, x, 255);
        drawPixel(image, Green, y, x, 0);
        drawPixel(image, Red, y, x, 255);

        //cout<<"("<<pointSet[i+numPoints][0]<<","<<pointSet[i+numPoints][1]<<")"<<endl;
    }

    //USE ML TO RECALCULATE COVARIANCE AND MEAN
    //for now, assume value must be between minimum and maximum x and y
    //P(D | params) = product k=1 to n of P(Xk | params)
        //x is individual data points
        //params set for each class
    //maximized ln(p(D | params))
    //mean should be sample mean x+x+x / n

    double classAtestMean[2] = {0,0};
    double classBtestMean[2] = {0,0};
    //run through class A to find mean of class A
    for(int i=0; i<numPoints; i++)
    {
        classAtestMean[0]+=pointSet[i][0];
        classAtestMean[1]+=pointSet[i][1];
    }
    for(int i=numPoints; i<numPoints*2; i++)
    {
        classBtestMean[0]+=pointSet[i][0];
        classBtestMean[1]+=pointSet[i][1];
    }

    meanA[0][0] = classAtestMean[0]/numPoints;
    meanA[1][0] = classAtestMean[1]/numPoints;

    meanB[0][0] = classBtestMean[0]/numPoints;
    meanB[1][0] = classBtestMean[1]/numPoints;

    cout<<"Mean for set A is calculated as: ("<<meanA[0][0]<<","<<meanA[1][0]<<")."<<endl;
    cout<<"Mean for set B is calculated as: ("<<meanB[0][0]<<","<<meanB[1][0]<<")."<<endl;

    //lets do covariance now
    double classAtestCovariance[2][2] = {{0,0},{0,0}};
    double classBtestCovariance[2][2] = {{0,0},{0,0}};
    for(int i=0; i<numPoints; i++)
    {
        double tempX = 0.0;
        double tempY = 0.0;

        //calculate distance from mean
        tempX = pointSet[i][0] - meanA[0][0];
        tempY = pointSet[i][1] - meanA[0][1];

        //square it
        tempX *= tempX;
        tempY *= tempY;

        classAtestCovariance[0][0]+=tempX;
        classAtestCovariance[1][1]+=tempY;
    }
    for(int i=numPoints; i<numPoints*2; i++)
    {
        double tempX = 0.0;
        double tempY = 0.0;

        //calculate distance from mean
        tempX = pointSet[i][0] - meanB[0][0];
        tempY = pointSet[i][1] - meanB[0][1];

        //square it
        tempX *= tempX;
        tempY *= tempY;

        classBtestCovariance[0][0]+=tempX;
        classBtestCovariance[1][1]+=tempY;
    }

    stdDevA[0][0] = sqrt(classAtestCovariance[0][0] / numPoints);
    stdDevA[1][1] = sqrt(classAtestCovariance[1][1] / numPoints);
    stdDevB[0][0] = sqrt(classBtestCovariance[0][0] / numPoints);
    stdDevB[1][1] = sqrt(classBtestCovariance[1][1] / numPoints);

    cout<<"Covariance for set A is calculated as: ("<<classAtestCovariance[0][0]/numPoints<<","<<classAtestCovariance[0][1]<<")."<<endl;
    cout<<"                                       ("<<classAtestCovariance[1][0]<<","<<classAtestCovariance[1][1]/numPoints<<")."<<endl;
    cout<<"Covariance for set B is calculated as: ("<<classBtestCovariance[0][0]/numPoints<<","<<classBtestCovariance[0][1]<<")."<<endl;
    cout<<"                                       ("<<classBtestCovariance[1][0]<<","<<classBtestCovariance[1][1]/numPoints<<")."<<endl;

    //run through our data again with the classifier, count if it gets calculated correctly or not
    for(int i=0; i<numPoints*2; i++)
    {
        double pAc = calcProbability(pointSet[i], meanA, stdDevA) * probabilityA;
        double pBc = calcProbability(pointSet[i], meanB, stdDevB) * probabilityB;

        if(pAc > pBc)
        {
            if(pointSet[i][2] == 1.0f)
            {
                incorrect ++;

                int x = (pointSet[i][0] * spread) + xOffset;
                int y = 500 - (pointSet[i][1] * spread) + yOffset;

                drawPixel(image, Blue, y, x, 0.0);
                drawPixel(image, Green, y, x, 0.0);
                drawPixel(image, Red, y, x, 255.0);
            }
            else
                correct ++;
        }
        else
        {
            if(pointSet[i][2] == 1.0f)
                correct ++;
            else
            {
                incorrect ++;

                int x = (pointSet[i][0] * spread) + xOffset;
                int y = 500 - (pointSet[i][1] * spread) + yOffset;

                drawPixel(image, Blue, y, x, 255.0);
                drawPixel(image, Green, y, x, 0.0);
                drawPixel(image, Red, y, x, 0.0);
            }
        }
    }


    cout<<"Correct classified: "<<correct<<endl;
    cout<<"Incorrect classified: "<<incorrect<<endl;

	// Access boundary points through chernoffBoundPoints, generate points for use
	//makeBound(chernoffBoundPoints,meanA,meanB,covarianceA,covarianceB);
/*
    //plot it
    for(int i=0; i<numPoints*2; i++)
    {
        int x = (chernoffBoundPoints[i][0] * chernoffScale) + 50;
        int y = 500 - (chernoffBoundPoints[i][1] * chernoffScale) + 0;

        drawPixel(image, Blue, x, y, 0.0);
        drawPixel(image, Green, x, y, 0.0);
        drawPixel(image, Red, x, y, 255.0);
    }cout<<"TEST"<<endl;

    findMinB(chernoffBoundPoints, minimumBeta, minimumY);

	cout << "The Beta that minimizes error for the Chernoff bound is: " << minimumBeta <<endl
		 << "Where e^-k(b) is : " << minimumY   << endl
		  <<"P(error), the Chernoff error bound, is equal to: "<< getProb(probabilityA,probabilityB,minimumY)<< endl<< endl;

	// calculating battacharyya bound
	kb = getBattacharayyaBound(meanA,meanB,covarianceA,covarianceB);

	cout << "The Bhattacharyya bound gives us k(0.5) = " << kb << endl
	   	<< "where e^-k(b) is: " << exp(-kb) << endl
		<<"P(error), the Bhattacharyya error bound, is equal to: " << getProb(probabilityA,probabilityB,exp(-kb)) << endl;

    // generate a graph with all error bounds
	makePlot(outputGraph, chernoffBoundPoints,minimumBeta, minimumY, exp(-kb));

    ofstream output;
    output.open(filepathRoot.c_str());

    for(int i=0; i<numPoints*2; i++)
    {
        for(int j=0; j<6; j++)
        {
            output<<outputGraph[i][j];

            if(j!=5)
                output<<'\t';
            else
                output<<endl;
        }
    }

    output.close();*/

    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);
}
