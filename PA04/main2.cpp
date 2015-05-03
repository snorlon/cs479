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

#include "libsvm-3.20/svm.h"
#include "libsvm-3.20/svm.cpp"

using namespace std;
using namespace cv;

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

const char *CSV_FILE = "/Users/kuanting/libsvm-3.17/heart_scale.csv";
const char *MODEL_FILE = "/Users/kuanting/libsvm-3.17/heart_scale.model";

int main(int argc, char * argv[])
{
    CvMLData dataFile;
    
    // Load matrix data in csv format
    if (dataFile.read_csv(CSV_FILE) != 0)
    {
        fprintf(stderr, "Can't read csv file %s\n", CSV_FILE);
        return -1;
    }
    
    Mat dataMat(dataFile.get_values()); // Default data type is float
    
    struct svm_model *SVMModel;
    if ((SVMModel = svm_load_model(MODEL_FILE)) == 0) {
        fprintf(stderr, "Can't load SVM model %s", MODEL_FILE);
        return -2;
    }
    
    struct svm_node *svmVec;
    svmVec = (struct svm_node *)malloc((dataMat.cols+1)*sizeof(struct svm_node));
    double *predictions = new double[dataMat.rows];
    float *dataPtr = dataMat.ptr<float>(); // Get data from OpenCV Mat
    double prob_est[2];  // Probability estimation
    int r, c;
    for (r=0; r<dataMat.rows; r++)
    {
        for (c=0; c<dataMat.cols; c++)
        {
            svmVec[c].index = c+1;  // Index starts from 1; Pre-computed kernel starts from 0
            svmVec[c].value = dataPtr[r*dataMat.cols + c];
        }
        svmVec[c].index = -1;   // End of line
        
        if(svm_check_probability_model(SVMModel))
        {
            predictions[r] = svm_predict_probability(SVMModel, svmVec, prob_est);
            printf("%f\t%f\t%f\n", predictions[r], prob_est[0], prob_est[1]);
        }
        else
        {
            predictions[r] = svm_predict(SVMModel, svmVec);
            printf("%f\n", predictions[r]);
        }
    }
    
    return 0;
}
