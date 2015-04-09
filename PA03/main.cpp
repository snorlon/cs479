#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <fstream>
#include <vector>
#include <errno.h>
#include <dirent.h>
#include "jacobi/jacobi.c"

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

int main(int argc, char** argv)
{

    string filepathRoot = "output.txt";

    vector<Mat> databaseRaw = vector<Mat>();
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
        //seed = atoi(argv[1]);
    }

    string dir = string("./Faces/fa_H");
    vector<string> files = vector<string>();

    getdir(dir,files);

    //sort files by their id (for our ease)
    std::sort(files.begin(), files.end());

    for (unsigned int i = 0;i < files.size();i++) {
        //add the image to our vector of Mats

        Mat tempImg = imread( dir + "/" + files[i], 1 );
        if(!tempImg.data)
        {
            cout << (dir + "/" + files[i]) << "FAILED" << endl;
            continue;//can't add a failed image
        }
        cv::cvtColor(tempImg, tempImg, cv::COLOR_BGR2GRAY);

        cout << files[i] << endl;
        
        databaseRaw.push_back(tempImg);
    }

    //test finding eigenvalues of the first image
    Mat testImage = databaseRaw.back();    
    width = (testImage.cols*testImage.rows)+1;
    height = databaseRaw.size()+1;
    int size = max(width, height)+1;
    double* eigenValues = new double[size];
    double** eigenVectors = new double*[size];

    double* averageFace = new double[width]; 
    double** covarianceMatrix = new double*[databaseRaw.size()+1]; 

    double** values = new double*[size];
    Mat averageFaceMat = databaseRaw.back().clone();

    for(int i=0; i<size; i++)
    {
        values[i] = new double[size];
        eigenVectors[i] = new double[size];

        eigenValues[i] = 0;
        averageFace[i] = 0;

        for(int j=0; j<size; j++)
        {
            values[i][j] = 0;
            eigenVectors[i][j] = 0;
        }
    }

    for(int i=0; i<databaseRaw.size()+1; i++)
    {
        covarianceMatrix[i] = new double[databaseRaw.size()+1];

        for(int j=0; j<databaseRaw.size()+1; j++)
        {
            covarianceMatrix[i][j] = 0;
        }
    }


    //store all of the images into the value array
    for(int index=0; index<databaseRaw.size(); index++)
    {
        Mat currMat = databaseRaw.at(index);
        for(int i=0; i<currMat.rows; i++)
        {
            for(int j=0; j<currMat.cols; j++)
            {
                values[index+1][(i*currMat.cols)+j+1] = currMat.at<uchar>(i,j);
            }
        }
    }


    //compute the mean of the faces
    for(int index=0; index<databaseRaw.size(); index++)
    {
        for(int i=0; i<width; i++)
        {
            averageFace[i+1] += values[index+1][i];
        }
    }

    for(int i=0; i<width; i++)
    {
        averageFace[i] /= databaseRaw.size();
    }

    //turn average face into an image
    for(int i=0; i<averageFaceMat.rows; i++)
    {
        for(int j=0; j<averageFaceMat.cols; j++)
        {
            averageFaceMat.at<uchar>(i,j) = averageFace[(i*averageFaceMat.cols)+j+1];
        }
    }

    //compute the covariance
    //assume that the current face data is A^T as it is already horizontal
    for(int i=0; i<databaseRaw.size(); i++)
    {
cout<<i<<endl;
        for(int j=i; j<databaseRaw.size(); j++)
        {
            //calculate product of the image differences
            double sum = 0;

            for(int x=0; x<width; x++)
            {
                for(int y=0; y<width; y++)
                {
                    sum+= (values[i][x] - averageFace[x])*(values[j][y] - averageFace[y]);
                }
            }

            covarianceMatrix[i+1][j+1] = sum;
            covarianceMatrix[j+1][i+1] = sum;
        }
    }
    
    for(int i=0; i<databaseRaw.size(); i++)
    {
        for(int j=0; j<databaseRaw.size(); j++)
        {
            covarianceMatrix[i+1][j+1] /= databaseRaw.size();
        }
    }

    cout<<"Calculated covariance matrix."<<endl;

    cout<<"Saving covariance matrix to covmatrix.txt"<<endl;

    ofstream output("covmatrix.txt");

    for(int i=0; i<databaseRaw.size(); i++)
    {
        for(int j=0; j<databaseRaw.size(); j++)
        {
            output<<covarianceMatrix[i+1][j+1]<<" ";
        }
        output<<endl;
    }

    output.close();
    

    //int a = jacobi(values,size-1, eigenValues,eigenVectors);
//cout<<a<<"|"<<endl;

    //print out eigenvectors?
    for(int i=1; i<size; i++)
    {
        //cout<<eigenValues[i]<<endl;
    }

    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    moveWindow("Display Image", 20, 20);
    imshow("Display Image", databaseRaw.front());

    namedWindow("Display Image2", CV_WINDOW_AUTOSIZE );
    moveWindow("Display Image2", 120, 20);
    imshow("Display Image2", averageFaceMat);

    waitKey(0);

    //cleanup
    databaseRaw.clear();
}
