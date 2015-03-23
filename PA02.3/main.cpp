#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

#define pi 3.14159265359

using namespace std;
using namespace cv;

int BLUE_INDEX = 0;//0
int GREEN_INDEX = 1;//1
int RED_INDEX = 2;//2

double bestThreshold = 0.020;
int bestErrorCount = 9999999;

bool useYCbCr = true;


double calcProbability(double point[2], double mean[1][2], double StDev[2][2])
{
    double nx = (point[0] - mean[0][0]) / StDev[0][0];//n
    double ny = (point[1] - mean[0][1]) / StDev[1][1];//n

    double PrX = (1/(2*pi)) * exp(-nx*nx / 2); //P(X | Class) = P(N = n)
    double PrY = (1/(2*pi)) * exp(-ny*ny / 2); //P(Y | Class)

    double PrXYgC = PrX * PrY;//P(X,Y | Class)

    return PrXYgC;
}

double calc_r(Mat image, int y, int x)
{
    //convert r g b to chromatic color space

    double R = (double)image.at<cv::Vec3b>(y,x)[RED_INDEX];
    double G = (double)image.at<cv::Vec3b>(y,x)[GREEN_INDEX];
    double B = (double)image.at<cv::Vec3b>(y,x)[BLUE_INDEX];

    double sum = R+G+B;

    if(!useYCbCr)
        return (R/(sum));
    else
        return (-0.169*R - 0.332*G + 0.500*B);
}

double calc_g(Mat image, int y, int x)
{
    //convert r g b to chromatic color space

    double R = (double)image.at<cv::Vec3b>(y,x)[RED_INDEX];
    double G = (double)image.at<cv::Vec3b>(y,x)[GREEN_INDEX];
    double B = (double)image.at<cv::Vec3b>(y,x)[BLUE_INDEX];

    double sum = R+G+B;

    if(!useYCbCr)
        return (G/(sum));
    else
        return (0.500*R - 0.419*G - 0.081*B);
}

int main(int argc, char** argv )
{

    //training and reference images
    Mat image1_t, image1_ref;
    Mat image3_t, image3_ref;
    Mat image6_t, image6_ref;

    image1_t = imread( "Data_Prog2/Training_1.ppm", 1 );
    image1_ref = imread( "Data_Prog2/ref1.ppm", 1 );

    image3_t = imread( "Data_Prog2/Training_3.ppm", 1 );
    image3_ref = imread( "Data_Prog2/ref3.ppm", 1 );

    image6_t = imread( "Data_Prog2/Training_6.ppm", 1 );
    image6_ref = imread( "Data_Prog2/ref6.ppm", 1 );

    int width_1 = image1_t.cols;
    int height_1 = image1_t.rows;
    int width_3 = image3_t.cols;
    int height_3 = image3_t.rows;
    int width_6 = image6_t.cols;
    int height_6 = image6_t.rows;

    string bonusTag = "Chromatic";

    if(useYCbCr)
        bonusTag = "useYCbCr";

    bonusTag = "data/roc" + bonusTag;

    bonusTag+= ".txt";
    
    ofstream output(bonusTag.c_str());

    if ( !image1_t.data || !image1_ref.data ||  !image3_t.data || !image3_ref.data ||  !image6_t.data || !image6_ref.data )
    {
        printf("No image data \n");
        return -1;
    }

    double mean[1][2] = {0.0,0.0};//[r,g]
    int skinCount = 0;//count of skin pixels

    //iterate across the pixels of the modeling image
    for(int i=0; i<height_1; i++)
    {
        for(int j=0; j<width_1; j++)
        {

            //only look at the points that have a red above 1 in the reference image
            if(image1_ref.at<cv::Vec3b>(i,j)[RED_INDEX] > 5.0)
            {

                //use these as sample points
                mean[0][0] += calc_r(image1_t, i, j);
                mean[0][1] += calc_g(image1_t, i, j);

                skinCount++;
            }
        }
    }

    //average them based on total num of points
    mean[0][0] /= skinCount;
    mean[0][1] /= skinCount;

    cout<<"Average mean r: "<<mean[0][0]<<endl;
    cout<<"Average mean g: "<<mean[0][1]<<endl;

    //calculate the covariance
    double covariance[2][2] = {{0.0,0.0},{0.0,0.0}};//[r,g]

    //iterate across the pixels of the modeling image
    for(int i=0; i<height_1; i++)
    {
        for(int j=0; j<width_1; j++)
        {
            //only look at the points that have a blue above 1 in the reference image
            if(image1_ref.at<cv::Vec3b>(i,j)[RED_INDEX] > 5.0)
            {

                double rDiff = calc_r(image1_t, i, j) - mean[0][0];
                double gDiff = calc_g(image1_t, i, j) - mean[0][1];

                //use these as sample points
                covariance[0][0] += rDiff*rDiff;
                covariance[0][1] += rDiff*gDiff;
                covariance[1][0] += gDiff*rDiff;
                covariance[1][1] += gDiff*gDiff;
            }
        }
    }

    //average them based on total num of points
    covariance[0][0] = (covariance[0][0] / (skinCount-1));
    covariance[0][1] = abs(covariance[0][1] / (skinCount-1));
    covariance[1][0] = abs(covariance[1][0] / (skinCount-1));
    covariance[1][1] = (covariance[1][1] / (skinCount-1));

    cout<<"Average covariance: "<<covariance[0][0]<<"  ,  "<<covariance[0][1]<<endl;
    cout<<"                    "<<covariance[1][0]<<"  ,  "<<covariance[1][1]<<endl;


    skinCount = 0;
    int nonSkinCount = 0;
    int falsePositiveCount = 0;
    int falseNegativeCount = 0;

    //sqrt covariance for st dev
    double standardDeviation[2][2];
    standardDeviation[0][0] = sqrt(covariance[0][0]);
    standardDeviation[0][1] = sqrt(abs(covariance[0][1]));
    standardDeviation[1][0] = sqrt(abs(covariance[1][0]));
    standardDeviation[1][1] = sqrt(covariance[1][1]);

    output<<"Threshold\tFalseNegativesImg3\tFalsePositivesImg3\tFalseNegativesImg6\tFalsePositivesImg6\tFalseNegativesSum\tFalsePositivesSum"<<endl;

    //test the testing images  
    //batch test 25 times
    for(double t=0.001; t<=0.025; t+=0.001)
    {
        int falsePositiveCountSet[3] = {0,0,0};
        int falseNegativeCountSet[3] = {0,0,0};

        //iterate across the pixels of the testing image 3
        for(int i=0; i<height_3; i++)
        {
            for(int j=0; j<width_3; j++)
            {
                bool isSkin = false;

                //convert r g b to chromatic color space
                double r = calc_r(image3_t, i, j);
                double g = calc_g(image3_t, i, j);

                //classify based on the r and g
                    double point[2] = {r,g};

                    //calculate chance that we are, in fact, skin
                    double probabilitySkin = calcProbability(point, mean, standardDeviation);

                    //threshold
                    if(probabilitySkin > t)
                        isSkin = true;

                //crosscheck classification with the real reference image
                //only look at the points that have a red above 1 in the reference image
                if(image3_ref.at<cv::Vec3b>(i,j)[RED_INDEX] > 5.0)
                {
                    if(!isSkin)
                        falseNegativeCountSet[0]++;
                }
                else
                {                
                    if(isSkin)
                        falsePositiveCountSet[0]++;
                }
                
            }
        }

        //iterate across the pixels of the testing image 6
        for(int i=0; i<height_6; i++)
        {
            for(int j=0; j<width_6; j++)
            {
                bool isSkin = false;

                //convert r g b to chromatic color space
                double r = calc_r(image6_t, i, j);
                double g = calc_g(image6_t, i, j);

                //classify based on the r and g
                    double point[2] = {r,g};

                    //calculate chance that we are, in fact, skin
                    double probabilitySkin = calcProbability(point, mean, standardDeviation);

                    //threshold
                    if(probabilitySkin > t)
                        isSkin = true;

                //crosscheck classification with the real reference image
                //only look at the points that have a red above 1 in the reference image
                if(image6_ref.at<cv::Vec3b>(i,j)[RED_INDEX] > 5.0)
                {
                    if(!isSkin)
                        falseNegativeCountSet[1]++;
                }
                else
                {                
                    if(isSkin)
                        falsePositiveCountSet[1]++;
                }
                
            }
        }

        //sum up the fp and fn's
        falseNegativeCountSet[2] = falseNegativeCountSet[0] + falseNegativeCountSet[1];
        falsePositiveCountSet[2] = falsePositiveCountSet[0] + falsePositiveCountSet[1];

        //print the data
        output<<t<<"\t"<<falseNegativeCountSet[0]<<"\t"<<falsePositiveCountSet[0]<<"\t"<<falseNegativeCountSet[1]<<"\t"<<falsePositiveCountSet[1]<<"\t"<<falseNegativeCountSet[2]<<"\t"<<falsePositiveCountSet[2]<<endl;

        //find the best total
        //value correct face more than incorrect
        double score = falseNegativeCountSet[2]*0.75 + falsePositiveCountSet[2]*0.25;
        if(score < bestErrorCount)
        {
            bestErrorCount = score;
            bestThreshold = t;
        }
    }

    //iterate across the pixels of the testing image
    for(int i=0; i<height_3; i++)
    {
        for(int j=0; j<width_3; j++)
        {
            //convert r g b to chromatic color space
            double r = calc_r(image3_t, i, j);
            double g = calc_g(image3_t, i, j);

            //classify based on the r and g
                double point[2] = {r,g};

                //calculate chance that we are, in fact, skin
                double probabilitySkin = calcProbability(point, mean, standardDeviation);

                //threshold
                if(probabilitySkin > bestThreshold)
                {
                    //make them BLEED on the image!
                    image3_t.at<cv::Vec3b>(i,j)[RED_INDEX] = 255;
                    image3_t.at<cv::Vec3b>(i,j)[BLUE_INDEX] = 0;
                    image3_t.at<cv::Vec3b>(i,j)[GREEN_INDEX] = 0;
                    
                }            
        }
    }

    for(int i=0; i<height_6; i++)
    {
        for(int j=0; j<width_6; j++)
        {
            bool isSkin = false;

            //convert r g b to chromatic color space
            double r = calc_r(image6_t, i, j);
            double g = calc_g(image6_t, i, j);

            //classify based on the r and g
                double point[2] = {r,g};

                //calculate chance that we are, in fact, skin
                double probabilitySkin = calcProbability(point, mean, standardDeviation);

                //threshold
                if(probabilitySkin > bestThreshold)
                {
                    skinCount++;
                    isSkin = true;

                    //make them BLEED on the image!
                    image6_t.at<cv::Vec3b>(i,j)[RED_INDEX] = 255;
                    image6_t.at<cv::Vec3b>(i,j)[BLUE_INDEX] = 0;
                    image6_t.at<cv::Vec3b>(i,j)[GREEN_INDEX] = 0;
                    
                }
                else
                {
                    nonSkinCount++;
                }

            //crosscheck classification with the real reference image
            //only look at the points that have a red above 1 in the reference image
            if(image6_ref.at<cv::Vec3b>(i,j)[RED_INDEX] > 5.0)
            {
                if(!isSkin)
                    falseNegativeCount++;
            }
            else
            {                
                if(isSkin)
                    falsePositiveCount++;
            }
            
        }
    }

    cout<<"Counted "<<skinCount<<" skin pixels!"<<endl;
    cout<<"Counted "<<nonSkinCount<<" non-skin pixels!"<<endl;
    cout<<"Counted "<<falsePositiveCount<<" false-positive pixels!"<<endl;
    cout<<"Counted "<<falseNegativeCount<<" false-negative pixels!"<<endl;
    cout<<"Best threshold is "<<bestThreshold<<"."<<endl;
    output<<"Best threshold is "<<bestThreshold<<"."<<endl;

    //DONE

    //resize(image6_t, image6_t, Size(), 0.5, 0.5, INTER_CUBIC);


    output.close();

    if(useYCbCr)
    {
        imwrite( "data/Image3YCbCrResult.png", image3_t );
        imwrite( "data/Image6YCbCrResult.png", image6_t );
    }
    else
    {
        imwrite( "data/Image3ChromaticResult.png", image3_t );
        imwrite( "data/Image6ChromaticResult.png", image6_t );
    }

    //namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    //imshow("Display Image", image6_t);

    //waitKey(0);

    return 0;
}
