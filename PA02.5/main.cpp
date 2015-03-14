#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace cv;

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

    if ( !image1_t.data || !image1_ref.data ||  !image3_t.data || !image3_ref.data ||  !image6_t.data || !image6_ref.data )
    {
        printf("No image data \n");
        return -1;
    }

    float sigma = 2;
    

    //clear our original image to fit new one
    /*double** sobel_result = new double*[height];
    for(int i=0; i<height; i++)
    {
        sobel_result[i] = new double[width];
        for(int j=0; j<width; j++)
            sobel_result[i][j] = 0.0;
    }


    //init mask
    double sobel_mask_dx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    double sobel_mask_dy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};

    int maskIndex = 1;
    int maskSize = 3;

    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
            image.at<cv::Vec3b>(i,j) = ((uchar) std::max(0.0, std::min((sobel_result[i][j] * 255), 255.0)));
    }*/

    double mean[1][2] = {0.0,0.0};//[r,g]
    int skinCount = 0;//count of skin pixels

    //iterate across the pixels of the modeling image
    for(int i=0; i<height_1; i++)
    {
        for(int j=0; j<width_1; j++)
        {
            int R = (float)image1_t.at<cv::Vec3b>(i,j)[0];
            int G = (float)image1_t.at<cv::Vec3b>(i,j)[1];
            int B = (float)image1_t.at<cv::Vec3b>(i,j)[2];
            double r = 0;
            double g = 0;

            //only look at the points that have a blue above 1 in the reference image
            if((float) image1_ref.at<cv::Vec3b>(i,j)[0] > 1)
            {
                //convert r g b to chromatic color space
                r = (double)(R/(double)(R+G+B));
                g = (double)(G/(double)(R+G+B));

                //use these as sample points
                mean[0][0] += r;
                mean[0][1] += g;

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
            int R = (float)image1_t.at<cv::Vec3b>(i,j)[0];
            int G = (float)image1_t.at<cv::Vec3b>(i,j)[1];
            int B = (float)image1_t.at<cv::Vec3b>(i,j)[2];
            double r = 0;
            double g = 0;

            //only look at the points that have a blue above 1 in the reference image
            if((float) image1_ref.at<cv::Vec3b>(i,j)[0] > 1)
            {
                //convert r g b to chromatic color space
                r = (double)(R/(double)(R+G+B));
                g = (double)(G/(double)(R+G+B));

                //get the covariance

                //use these as sample points
                covariance[0][0] += (r - mean[0][0])*(r - mean[0][0]);
                covariance[0][1] += (r - mean[0][0])*(g - mean[0][1]);
                covariance[1][0] += (g - mean[0][1])*(r - mean[0][0]);
                covariance[1][1] += (g - mean[0][1])*(g - mean[0][1]);

                skinCount++;
            }
        }
    }

    //average them based on total num of points
    covariance[0][0] /= skinCount;
    covariance[0][1] /= skinCount;
    covariance[1][0] /= skinCount;
    covariance[1][1] /= skinCount;


    cout<<"Average covariance: "<<covariance[0][0]<<"  ,  "<<covariance[0][1]<<endl;
    cout<<"                    "<<covariance[1][0]<<"  ,  "<<covariance[1][1]<<endl;

    //DONE



    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    imshow("Display Image", image1_t);

    waitKey(0);

    return 0;
}
