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

void calculateEigenCoefs(double outputMatrix[], double test_image[], double average_face[], vector<Mat> eigenFaces,int faceCount, int height, int width)
{
cout<<"TOAST"<<faceCount<<endl;
    double* difference_face = new double[faceCount];
cout<<"ROT"<<faceCount<<endl;

    for(int i=0; i<width*height; i++)
    {
        difference_face[i] = (test_image[i] - average_face[i])/255.0;
    }
cout<<"TOT"<<faceCount<<endl;

    for(int i=0; i< faceCount; i++)
    {
        outputMatrix[i] = 0;
    }
cout<<"TIT"<<faceCount<<endl;

    cout<<"calculating coefficient"<<endl;

    //for each i in outputMatrix, it should be the multiplied product of every single image by the test image
    //math it out bro!
    for(int i=0; i<faceCount; i++)
    {
        for(int j=0; j<height; j++)
        {
            for(int k=0; k<width; k++)
            {
                //averageFaceMat.at<uchar>(i,j) = averageFace[(i*averageFaceMat.cols)+j+1];
                outputMatrix[i] += ((eigenFaces.at(i).at<uchar>(j,k))/255.0) * difference_face[j*(width)+k];
            }
        }
    }

    //normalize the eigen mults
    double largestEigenSum = 0;
    for(int i=0; i<faceCount; i++)
        largestEigenSum += outputMatrix[i];
    for(int i=0; i<faceCount; i++)
        outputMatrix[i] /= largestEigenSum;


cout<<"??"<<faceCount<<endl;
    cout<<"Done calculating. They are: "<<endl;
    for(int i=0; i<faceCount; i++)
    {
        cout<<outputMatrix[i]<<" |";
    }

    cout<<endl;

    //delete[] difference_face;

cout<<"NOT HERE PAPI"<<endl;
}

int main(int argc, char** argv)
{

    string filepathRoot = "output.txt";

    vector<Mat> databaseRaw = vector<Mat>();

    int mode = 5;

    if(argc > 1)
    {
        //use the first parameter as a seed for replicating results
        //seed = atoi(argv[1]);
    }

    string dir = string("./Faces/fa_H");
    string dir2 = string("./Faces/fb_H");
    vector<string> files = vector<string>();
    vector<string> files2 = vector<string>();

    getdir(dir,files);
    getdir(dir2,files2);

    //sort files by their id (for our ease)
    std::sort(files.begin(), files.end());
    std::sort(files2.begin(), files2.end());

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
        
        databaseRaw.push_back(tempImg.clone());
        tempImg.release();
    }
//return 0;

    //test finding eigenvalues of the first image
    Mat testImage = databaseRaw.back();    
    int imageTotal = (testImage.cols*testImage.rows);
    int height = databaseRaw.size();

    double averageFace[imageTotal]; 

    double** values[height];
    for(int i=0; i<height; i++)
    {
        values[i] = new double*[testImage.rows];
        for(int j=0; j<testImage.rows; j++)
            values[i][j] = new double[testImage.cols];
    }

    Mat averageFaceMat = databaseRaw.back().clone();
    Mat testFaceMat = databaseRaw.back().clone();

    for(int i=0; i<height; i++)
    {
        for(int j=0; j<testImage.rows; j++)
        {
            for(int k=0; k<testImage.cols; k++)
                values[i][j][k] = 0;
        }
    }

    for(int i=0; i<imageTotal; i++)
        averageFace[i] = 0;

    //store all of the images into the value array
    for(int index=0; index<databaseRaw.size(); index++)
    {
        Mat currMat = databaseRaw.at(index);
        for(int i=0; i<currMat.rows; i++)
        {
            for(int j=0; j<currMat.cols; j++)
            {
                values[index][i][j] = currMat.at<uchar>(i,j);
                if((i*currMat.cols)+j == 205 && index==0)
                {
                    cout<<"TEST"<<values[index][i][j]<<endl;
                }
            }
        }
    }

    cout<<"TEST1"<<values[0][205]<<endl;
        
    //compute the mean of the faces
    for(int index=0; index<databaseRaw.size(); index++)
    {
        for(int i=0; i<testImage.rows; i++)
        {
            for(int j=0; j<testImage.cols; j++)
                averageFace[i*testImage.cols + j] += values[index][i][j];
        }
    }

    for(int i=0; i<imageTotal; i++)
    {
        averageFace[i] /= databaseRaw.size();
    }

    //turn average face into an image
    for(int i=0; i<averageFaceMat.rows; i++)
    {
        for(int j=0; j<averageFaceMat.cols; j++)
        {
            averageFaceMat.at<uchar>(i,j) = averageFace[(i*averageFaceMat.cols)+j];
        }
    }

    cout<<"TEST2"<<values[0][205]<<endl;

    /*namedWindow("Display Image2", CV_WINDOW_AUTOSIZE );
    moveWindow("Display Image2", 120, 20);
    imshow("Display Image2", averageFaceMat);

    waitKey(0);*/

    Mat data = asRowMatrix(databaseRaw, CV_32FC1);
    
    PCA pca(data, Mat(), CV_PCA_DATA_AS_ROW);

    Mat eigenvectors = pca.eigenvectors.clone();
    Mat eigenvalues = pca.eigenvalues.clone();

    if(mode == 4)
    {

        for(int i=1194; i<1204; i++)
        {
            ostringstream output;
            output<<"PC"<<i;

            string name = output.str();
            namedWindow(name, CV_WINDOW_AUTOSIZE );
            moveWindow(name, 90*(i-1194) + 100, 20);
            imshow(name,norm_0_255(pca.eigenvectors.row(i)).reshape(1, databaseRaw[0].rows));
        }
        waitKey(0);
    }

    cout<<"TEST3"<<values[0][205]<<endl;

    if(mode == 5)//calculate cutoff for the top 80%
    {
        cout<<"Calculating eigen sum"<<endl;
        float eigenSum = 0;
        float eigenSumCurr = 0;
        for(int i=0; i<height; i++)
        {
            eigenSum += eigenvalues.at<float>(0,i);
        }
        cout<<"EigenSum "<<eigenSum<<endl;

        eigenSumCurr = eigenSum*0.95;
        int cutOff = 0;

    cout<<eigenvalues.at<float>(0,0)<<endl;

        //calculate what would be 80% of it
        for(int i=0; i<height && eigenSumCurr > 0; i++)
        {
            eigenSumCurr -= eigenvalues.at<float>(0,i);
            //i have no idea what this should be doing
            cutOff++;
        }

        cout<<"Use the best "<<cutOff<<" eigenvectors."<<endl;
    }

    cout<<"TEST4"<<values[0][205]<<endl;

    vector<Mat> eigenFacesReduced = vector<Mat>();

    if(mode == 6)
    {
cout<<"A"<<endl;
        //by here we found that we found that the top 36 eigenfaces hold 80% of the data, 49 90%, 114 95%

        //attempt to recreate the average face from the reduced data
        int maxFaces = 36;//new M

cout<<"D"<<endl;


    cout<<"TEST5"<<values[0][205]<<endl;
        //store the eigenfaces into the thing
        for(int i=0; i<maxFaces; i++)
        {
            eigenFacesReduced.push_back(norm_0_255(pca.eigenvectors.row(i)).reshape(1, databaseRaw[0].rows));
            //save the faces
            imwrite(format("%s/eigenface_%d.png", "eigenfaces", i), norm_0_255(pca.eigenvectors.row(i)).reshape(1, databaseRaw[0].rows));
        }

 
    cout<<"TEST6"<<values[0][205]<<endl;
cout<<"B"<<endl;

        //calculate MxN^2 multiplied with N^2x1(the test image)
        double eigenvector_coefficients[maxFaces];

//void calculateEigenCoefs(double outputMatrix[], double test_image[], double average_face[], vector<Mat> eigenFaces,const int faceCount, int height, int width)
        //calculateEigenCoefs(eigenvector_coefficients, values[0], averageFace, eigenFacesReduced, maxFaces, testImage.cols,testImage.rows);
        double difference_face[maxFaces];

        for(int i=0; i<testImage.rows; i++)
        {
            for(int j=0; j<testImage.cols; j++)
            {
cout<<i<<"|"<<j<<endl;
                difference_face[i] = (values[0][i][j] - averageFace[i]);
            }
        }

cout<<"F"<<endl;
        for(int i=0; i< maxFaces; i++)
            eigenvector_coefficients[i] = 0;

        //for each i in outputMatrix, it should be the multiplied product of every single image by the test image
        for(int i=0; i<maxFaces; i++)
            for(int j=0; j<testImage.rows; j++)
                for(int k=0; k<testImage.cols; k++)
                    //averageFaceMat.at<uchar>(i,j) = averageFace[(i*averageFaceMat.cols)+j+1];
                    eigenvector_coefficients[i] += ((eigenFacesReduced.at(i).at<uchar>(j,k))/255.0) * difference_face[j*(testImage.cols)+k];

        //normalize the eigen mults
        /*double largestEigenSum = 0;
        for(int i=0; i<maxFaces; i++)
            largestEigenSum += eigenvector_coefficients[i];
        for(int i=0; i<maxFaces; i++)
        {
            eigenvector_coefficients[i] /= largestEigenSum;
    cout<<eigenvector_coefficients[i]<<endl;
        }*/


cout<<"STEP0"<<endl;
        Mat newImage = testImage.clone();
cout<<"STEP0.5"<<endl;
        double* nvalues[testImage.rows];
cout<<"STEP7.5"<<endl;
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
                    nvalues[i][j]+=eigenvector_coefficients[k]*((eigenFacesReduced.at(k).at<uchar>(i,j)));
        }

        //copy to the image
        for(int i=0; i<testImage.rows; i++)
            for(int j=0; j<testImage.cols; j++)
                newImage.at<uchar>(i,j) = nvalues[i][j];

        //add the average
        for(int i=0; i<testImage.rows; i++)
            for(int j=0; j<testImage.cols; j++)
                newImage.at<uchar>(i,j) += averageFaceMat.at<uchar>(i,j);

        //clamp it to 0 to 255
        newImage = norm_0_255(newImage);

cout<<"STEP4"<<endl;

        namedWindow("eigenvector", CV_WINDOW_AUTOSIZE );
        imshow("eigenvector",newImage);
        waitKey(0);
    }

    cout<<"Please enter an eigenvector index to view."<<endl;
imwrite(format("%s/avg_eigenface.png", "eigenfaces"), averageFaceMat);

    //wait until quit
    string input = "";
    while(input.compare("quit")!=0 && 0)
    {
        cin>>input;

        //allow looking at specific eigenfaces
        if(input.compare("A")==0)
        {

        }
        else if(input.compare("quit")==0)
        {

        }
        else//default is to try to look at a specific eigenvector
        {
            int value = stoi(input.c_str());

            if(value >= 0 && value < height)
            {
                //display that eigenvector
                namedWindow("eigenvector", CV_WINDOW_AUTOSIZE );
                imshow("eigenvector",norm_0_255(pca.eigenvectors.row(value)).reshape(1, databaseRaw[0].rows));
            }
        }
        waitKey(0);
        //destroyWindow("eigenvector");
    }

    cout<<"SHUTTING DOWN PROGRAM"<<endl;

    //cleanup
    databaseRaw.clear();
    files.clear();
    files2.clear();
}
