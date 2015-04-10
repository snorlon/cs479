#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/highgui/highgui.hpp>
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
    Mat image;

    image = imread( "graph.png", 1 );

    int width = image.cols;
    int height = image.rows;

    int mode = 4;//0 does nothing, 1 is for creating covariance matrix, 2 is for creating eigenvalues, 3 is for creating database of scalers for images, 4 is classification

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
    vector<int> databaseLabels;

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
    double* eigenValues = new double[databaseRaw.size()+1];
    double** eigenVectors = new double*[databaseRaw.size()+1];

    double* averageFace = new double[width]; 
    double** covarianceMatrix = new double*[databaseRaw.size()+1]; 

    double** values = new double*[size];
    Mat averageFaceMat = databaseRaw.back().clone();
    Mat testFaceMat = databaseRaw.back().clone();

    for(int i=0; i<size; i++)
    {
        values[i] = new double[size];

        for(int j=0; j<size; j++)
        {
            values[i][j] = 0;
        }
    }

    for(int i=0; i<width; i++)
        averageFace[i] = 0;

    for(int i=0; i<databaseRaw.size()+1; i++)
    {cout<<i<<endl;
        covarianceMatrix[i] = new double[databaseRaw.size()+1];
        eigenVectors[i] = new double[databaseRaw.size()+1];
        eigenValues[i] = 0;
        for(int j=0; j<databaseRaw.size()+1; j++)
        {
            covarianceMatrix[i][j] = 0;
            eigenVectors[i][j] = 0;
        }
    }
cout<<"TEST"<<endl;

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

    //find our covariance matrix
    if(mode == 1)
    {

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
    }

    if(mode == 2)
    {
        //load in covmatrix.txt from data/
        ifstream input("data/covmatrix.txt");

        double biggest = 0;

        for(int i=0; i<databaseRaw.size(); i++)
        {
            for(int j=0; j<databaseRaw.size(); j++)
            {
                double t = 0;
                input>>t;

                covarianceMatrix[i+1][j+1] = t;

                if(t > biggest)
                    biggest = t;
            }
        }

        input.close();

        //normalize the covariance matrix
        for(int i=0; i<databaseRaw.size(); i++)
        {
            for(int j=0; j<databaseRaw.size(); j++)
            {
                covarianceMatrix[i+1][j+1]/=biggest;
            }

        }

        //find dem eigenvalues
        int a = jacobi(covarianceMatrix,databaseRaw.size(), eigenValues,eigenVectors);
cout<<a<<"|"<<endl;

        //print out the eigenvectors
        ofstream output("eigenvectors.txt");

        for(int i=0; i<databaseRaw.size(); i++)
        {
            for(int j=0; j<databaseRaw.size(); j++)
            {
                output<<eigenVectors[i+1][j+1]<<" ";
            }
            output<<eigenValues[i];

            output<<endl;
        }

        output.close();

cout<<"Saved eigenvectors to eigenvectors.txt along with eigenvalues."<<endl;
    }
    if(mode == 3)
    {
        //load in the eigenvectors and eigenvalues
        //print out the eigenvectors
        ifstream input("data/eigenvectors.txt");

        for(int i=0; i<databaseRaw.size(); i++)
        {
            for(int j=0; j<databaseRaw.size(); j++)
            {
                input>>eigenVectors[i+1][j+1];
            }
            input>>eigenValues[i];
        }

        input.close();

        //turn first eigenvector into an image (for science)
        for(int i=0; i<testFaceMat.rows; i++)
        {
            for(int j=0; j<testFaceMat.cols; j++)
            {
                testFaceMat.at<uchar>(i,j) = averageFace[(i*testFaceMat.cols)+j+1] * eigenVectors[12][(i*testFaceMat.cols)+j+1];

                //product it out yo!
                //N^2 x1 * 1xM

                //sum up the values
            }
        }
    }
    if(mode == 4)
    {
        Mat data = asRowMatrix(databaseRaw, CV_32FC1);
        
        PCA pca(data, Mat(), CV_PCA_DATA_AS_ROW, 5);

        Mat eigenvectors = pca.eigenvectors.clone();

        imshow("pc1",norm_0_255(pca.eigenvectors.row(0)).reshape(1, databaseRaw[0].rows));
        imshow("pc2",norm_0_255(pca.eigenvectors.row(1)).reshape(1, databaseRaw[0].rows));
        imshow("pc3",norm_0_255(pca.eigenvectors.row(2)).reshape(1, databaseRaw[0].rows));
    }


    //print out eigenvectors?
    for(int i=1; i<size; i++)
    {
        //cout<<eigenValues[i]<<endl;
    }

    namedWindow("Display Image", CV_WINDOW_AUTOSIZE );
    moveWindow("Display Image", 20, 20);
    imshow("Display Image", testFaceMat);

    namedWindow("Display Image2", CV_WINDOW_AUTOSIZE );
    moveWindow("Display Image2", 120, 20);
    imshow("Display Image2", averageFaceMat);

    waitKey(0);

    delete[] eigenValues;

    for(int i=0; i<databaseRaw.size()+1; i++)
        delete[] eigenVectors[i];
    delete[] eigenVectors;

    for(int i=0; i<databaseRaw.size()+1; i++)
        delete[] covarianceMatrix[i];
    delete[] covarianceMatrix;

    //cleanup
    databaseRaw.clear();
}
