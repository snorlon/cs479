#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <fstream>
#include <vector>
#include <errno.h>
#include <dirent.h>
#include <string>
#include <sstream>

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



// Normalizes a given image into a value range between 0 and 255.
Mat norm_0_255(const Mat& src) {
    // Create and return normalized image:
    Mat dst;
    switch(src.channels()) {
    case 1:
        cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
        break;
    case 3:
        cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
        break;
    default:
        src.copyTo(dst);
        break;
    }
    return dst;
}

/*function... might want it in some class?*/
//from http://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/
int getdir (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

// Converts the images given in src into a row matrix.
Mat asRowMatrix(const vector<Mat>& src, int rtype, double alpha = 1, double beta = 0) {
    // Number of samples:
    size_t n = src.size();
    // Return empty matrix if no matrices given:
    if(n == 0)
        return Mat();
    // dimensionality of (reshaped) samples
    size_t d = src[0].total();
    // Create resulting data matrix:
    Mat data(n, d, rtype);
    // Now copy data:
    for(int i = 0; i < n; i++) {
        //
        if(src[i].empty()) {
            string error_message = format("Image number %d was empty, please check your input data.", i);
            CV_Error(CV_StsBadArg, error_message);
        }
        // Make sure data can be reshaped, throw a meaningful exception if not!
        if(src[i].total() != d) {
            string error_message = format("Wrong number of elements in matrix #%d! Expected %d was %d.", i, d, src[i].total());
            CV_Error(CV_StsBadArg, error_message);
        }
        // Get a hold of the current row:
        Mat xi = data.row(i);
        // Make reshape happy by cloning for non-continuous matrices:
        if(src[i].isContinuous()) {
            src[i].reshape(1, 1).convertTo(xi, rtype, alpha, beta);
        } else {
            src[i].clone().reshape(1, 1).convertTo(xi, rtype, alpha, beta);
        }
    }
    return data;
}

int main(int argc, char** argv)
{

    string filepathRoot = "output.txt";

    vector<Mat> databaseRaw = vector<Mat>();
    vector<Mat> eigenfaces = vector<Mat>();

    int mode = 6;
    int imgIndex = 0;

    if(argc > 1)
    {
        //use the first parameter as a seed for replicating results
        //seed = atoi(argv[1]);
        imgIndex = atoi(argv[1]);
    }

    string dir = string("./Faces/fa_H");
    string dir2 = string("./Faces/fb_H");
    string dir3 = string("./eigenfaces");
    vector<string> files = vector<string>();
    vector<string> files2 = vector<string>();
    vector<string> files3 = vector<string>();

    getdir(dir,files);
    getdir(dir2,files2);
    getdir(dir3,files3);

    //sort files by their id (for our ease)
    std::sort(files.begin(), files.end());
    std::sort(files2.begin(), files2.end());
    std::sort(files3.begin(), files3.end());

    for (unsigned int i = 0;i < files.size();i++) {
        //add the image to our vector of Mats

        Mat tempImg = imread( dir + "/" + files[i], 1 );
        if(!tempImg.data)
        {
            cout << (dir + "/" + files[i]) << "FAILED" << endl;
            continue;//can't add a failed image
        }
        cv::cvtColor(tempImg, tempImg, cv::COLOR_BGR2GRAY);

        //cout << files[i] << endl;
        
        databaseRaw.push_back(tempImg.clone());
        tempImg.release();
    }

    for (unsigned int i = 0;i < files3.size();i++) {
        //add the image to our vector of Mats

        Mat tempImg = imread( dir3 + "/" + files3[i], 1 );
        if(!tempImg.data)
        {
            cout << (dir3 + "/" + files3[i]) << "FAILED" << endl;
            continue;//can't add a failed image
        }
        cv::cvtColor(tempImg, tempImg, cv::COLOR_BGR2GRAY);

        //cout << files3[i] << endl;
        
        eigenfaces.push_back(tempImg.clone());
        tempImg.release();
    }
//return 0;

    //test finding eigenvalues of the first image
    Mat testImage = databaseRaw.back();
    int height = databaseRaw.size();

    Mat averageFaceMat = imread( "avg_eigenface.png", 1 );
    if(!averageFaceMat.data)
    {
        cout << "avg_eigenface.png FAILED" << endl;
        return 0;
    }
    cv::cvtColor(averageFaceMat, averageFaceMat, cv::COLOR_BGR2GRAY);

    /*namedWindow("Display Image2", CV_WINDOW_AUTOSIZE );
    moveWindow("Display Image2", 120, 20);
    imshow("Display Image2", averageFaceMat);

    waitKey(0);*/

    if(mode == 6)
    {
        //by here we found that we found that the top 36 eigenfaces hold 80% of the data, 49 90%, 114 95%

        //attempt to recreate the average face from the reduced data
        int maxFaces = eigenfaces.size();//new M
cout<<maxFaces<<endl;

        //calculate MxN^2 multiplied with N^2x1(the test image)
        double eigenvector_coefficients[maxFaces];

        for(int i=0; i< maxFaces; i++)
            eigenvector_coefficients[i] = 0;

        //convert the test image
        Mat tempFaceMat;
        databaseRaw.at(imgIndex).convertTo(tempFaceMat, CV_32FC1);
        Mat avgFaceMat;
        averageFaceMat.convertTo(avgFaceMat, CV_32FC1);

        //for each i in outputMatrix, it should be the multiplied product of every single image by the test image
        for(int i=0; i<maxFaces; i++)
        {
            for(int j=0; j<testImage.rows; j++)
                for(int k=0; k<testImage.cols; k++)
                {
                    eigenvector_coefficients[i] += (eigenfaces.at(i).at<char>(j,k)) * (tempFaceMat.at<float>(j,k) - avgFaceMat.at<float>(j,k));
                }
        }

        
        //normalize the eigen mults
        double largestEigenSum = 0;
        for(int i=0; i<maxFaces; i++)
            largestEigenSum += eigenvector_coefficients[i];
        for(int i=0; i<maxFaces; i++)
        {
            eigenvector_coefficients[i] /= largestEigenSum;
            cout<<eigenvector_coefficients[i]<<endl;
        }


        Mat newImage = tempFaceMat.clone();
        double* nvalues[testImage.rows];
        for(int i=0; i<testImage.rows; i++)
            nvalues[i] = new double[testImage.cols]();


        for(int i=0; i<testImage.rows; i++)
            for(int j=0; j<testImage.cols; j++)
                nvalues[i][j] = 0;

        //multiply out the eigenfaces
        for(int k=0; k<maxFaces; k++)
        {
            for(int i=0; i<testImage.rows; i++)
                for(int j=0; j<testImage.cols; j++)
                    nvalues[i][j]+=(eigenvector_coefficients[k])*((eigenfaces.at(k).at<char>(i,j)));
        }

        //reduce this to 0-255

        //add the average

        for(int i=0; i<testImage.rows; i++)
            for(int j=0; j<testImage.cols; j++)
                nvalues[i][j] += averageFaceMat.at<char>(i,j);

        //copy to the image CHECKED
        for(int i=0; i<testImage.rows; i++)
            for(int j=0; j<testImage.cols; j++)
            {
                cout<<nvalues[i][j]<<endl;
                newImage.at<float>(i,j) = nvalues[i][j]/255.0;
            }

        newImage = norm_0_255(newImage);

        namedWindow("eigenvectorc", CV_WINDOW_AUTOSIZE );
        moveWindow("eigenvectorc", 260, 60);
        imshow("eigenvectorc",averageFaceMat);

        namedWindow("eigenvector", CV_WINDOW_AUTOSIZE );
        moveWindow("eigenvector", 160, 60);
        imshow("eigenvector",newImage);

        namedWindow("eigenvectorb", CV_WINDOW_AUTOSIZE );
        moveWindow("eigenvectorb", 60, 60);
        imshow("eigenvectorb",databaseRaw.at(imgIndex));

        waitKey(0);
    }

    cout<<"SHUTTING DOWN PROGRAM"<<endl;

    //cleanup
    databaseRaw.clear();
    eigenfaces.clear();
    files.clear();
    files2.clear();
    files3.clear();
}
