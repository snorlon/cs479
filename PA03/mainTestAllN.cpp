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
    cv::normalize(src, dst, 0, 255, NORM_MINMAX, CV_8UC1);

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

    vector<Mat> databaseCombined = vector<Mat>();
    vector<Mat> eigenfaces = vector<Mat>();

    const int maxN = 50;

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
        
        databaseCombined.push_back(tempImg.clone());
        tempImg.release();
    }

    for (unsigned int i = 0;i < files2.size();i++) {
        //add the image to our vector of Mats

        Mat tempImg = imread( dir2 + "/" + files2[i], 1 );
        if(!tempImg.data)
        {
            cout << (dir2 + "/" + files2[i]) << "FAILED" << endl;
            continue;//can't add a failed image
        }
        cv::cvtColor(tempImg, tempImg, cv::COLOR_BGR2GRAY);

        //cout << files[i] << endl;
        
        databaseCombined.push_back(tempImg.clone());
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

    //test finding eigenvalues of the first image
    Mat testImage = databaseCombined.back();
    int height = databaseCombined.size();

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


    //by here we found that we found that the top 36 eigenfaces hold 80% of the data, 49 90%, 114 95%

    //attempt to recreate the average face from the reduced data
    int maxFaces = eigenfaces.size();//new M
cout<<"TEST"<<endl;

    static double database[1204][1204];

    //load the database
    ifstream input("data/database.txt");
    cout<<"started loading database."<<endl;

    for(int i=0; i<1204; i++)
        for(int j=0; j<1204; j++)
            input>>database[i][j];

    input.close();

    cout<<"database loaded."<<endl;

    int correct[maxN];
    int correct1[maxN];
    int correct2[maxN];

    for(int i=0; i<maxN; i++)
    {
        correct[i] = 0;
        correct1[i] = 0;
        correct2[i] = 0;
    }

    //iterate across our test images
    for(int j=0; j<databaseCombined.size(); j++)
    {

        //calculate MxN^2 multiplied with N^2x1(the test image)
        double eigenvector_coefficients[maxFaces];

        for(int i=0; i< maxFaces; i++)
            eigenvector_coefficients[i] = 0;

        //convert the test image
        Mat tempFaceMat;
        databaseCombined.at(j).convertTo(tempFaceMat, CV_32FC1);
        Mat avgFaceMat;
        averageFaceMat.convertTo(avgFaceMat, CV_32FC1);

        //for each i in outputMatrix, it should be the multiplied product of every single image by the test image
        for(int i=0; i<maxFaces; i++)
        {
            for(int m=0; m<testImage.rows; m++)
                for(int k=0; k<testImage.cols; k++)
                {
                    eigenvector_coefficients[i] += (eigenfaces.at(i).at<uchar>(m,k) - 128) * (tempFaceMat.at<float>(m,k) - avgFaceMat.at<float>(m,k));
                }
        }


        int bestWindow[maxN];
        double bestDist[maxN];

        for(int i=0; i<maxN; i++)
        {
            bestWindow[i] = -1;
            bestDist[i] = 99999999999;
        }

        for(int i=0; i<1204; i++)//check the database for matching
        {
            //calculate distance
            double distance = 0;

            for(int k=0; k< maxFaces; k++)
                distance += abs(floor(eigenvector_coefficients[k] - database[i][k]));

            distance = sqrt(distance);

            //check if this is a better distance for our best-match from the database
            for(int m=0; m<maxN; m++)
            {
                if(distance < bestDist[m])
                {
                    //shift down all of the ones after us
                    for(int p=maxN-2; p>=m && p>=0; p--)
                    {
                        bestWindow[p+1] = bestWindow[p];
                        bestDist[p+1] = bestDist[p];
                    }

                    bestWindow[m] = i+1;
                    bestDist[m] = distance;
                    break;//can escape if we found a better one
                }
            }

            //cout<<j<<" has distance of "<<distance<<" to "<<i<<endl;
        }

        //try to match for all of the n

        for(int q=1; q<=maxN; q++)
        {
            //add to the correct count if it found the proper image in the window it had
            for(int p=0; p<q; p++)
            {
                //check by id of the image
                string id1 = "";
                string id2 = "";

                if(bestWindow[p] <= 1204)
                    id1 = files.at(bestWindow[p]-1).substr(0,5);
                if(j >= 1204)
                    id2 = files2.at(j-1204).substr(0,5);
                if(j < 1204)
                    id2 = files.at(j).substr(0,5);

                if(id1.compare(id2) == 0)
                {
                    correct[q-1]++;
                    if(j<1204)
                        correct1[q-1]++;
                    else
                        correct2[q-1]++;
                    break;
                }
            }
        }

        tempFaceMat.release();
        avgFaceMat.release();
    }

    ofstream output("data/classification_numbers.txt");

    output<<"N\tCorrect Total\tCorrect Training\tCorrect Testing"<<endl;

    for(int i=0; i<maxN; i++)
    {
        cout<<"N = "<<i+1<<endl;
        cout<<correct[i]<<" classified correctly."<<endl;
        cout<<correct1[i]<<", "<<((int) (correct1[i]/12.04))<<"%, of the training set classified correctly."<<endl;
        cout<<correct2[i]<<", "<<((int) (correct2[i]/11.96))<<"%, of the test set were classified correctly."<<endl<<endl;

        output<<i+1<<"\t"<<correct[i]<<"\t"<<correct1[i]<<"\t"<<correct2[i]<<endl;
    }

    cout<<"Done"<<endl;

    output.close();

    /*namedWindow("eigenvectord", CV_WINDOW_AUTOSIZE );
    moveWindow("eigenvectord", 360, 60);
    imshow("eigenvectord",averageFaceMat);

    namedWindow("eigenvector", CV_WINDOW_AUTOSIZE );
    moveWindow("eigenvector", 160, 60);
    imshow("eigenvector",newImage);

    namedWindow("eigenvectorc", CV_WINDOW_AUTOSIZE );
    moveWindow("eigenvectorc", 260, 60);
    imshow("eigenvectorc",newImageb);

    namedWindow("eigenvectorb", CV_WINDOW_AUTOSIZE );
    moveWindow("eigenvectorb", 60, 60);
    imshow("eigenvectorb",databaseRaw.at(imgIndex));

    waitKey(0);*/

    cout<<"SHUTTING DOWN PROGRAM"<<endl;

    //cleanup
    databaseCombined.clear();
    eigenfaces.clear();
    files.clear();
    files2.clear();
    files3.clear();
}
