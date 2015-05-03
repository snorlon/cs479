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
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

#include "libsvm-3.20/svm.h"
#include "libsvm-3.20/svm.cpp"

using namespace std;
using namespace cv;

struct svm_parameter parameters;     // set by parse_command_line
struct svm_problem prob;        // set by read_problem
struct svm_model *model;
struct svm_node *x_space;

vector<vector<double>> loadPCA(string filename)
{
    vector<vector<double>> returnVector = vector<vector<double>>();

    ifstream input;
    double tempVal;
    string dummyString;

    input.open(filename);
    input>>tempVal;
    do{
        if(input.good()){
            //create a new set of data to push to
            vector<double> rowData = vector<double>();

            for(int i=0; i<30; i++){
                //prevents using a bad value
                if(input.good())
                    rowData.push_back(tempVal);

                input>>tempVal;}

            //clear out all after the first 30 eigen coefficients
            getline (input,dummyString);

            //add it to our data thing
            if(input.good())
                returnVector.push_back(rowData);
        }}
    while(input.good());
    input.close();

    return returnVector;
}

int main(int argc, char * argv[])
{
    parameters.svm_type = C_SVC;
    parameters.kernel_type = RBF;
    parameters.degree = 3;
    parameters.gamma = 0.5;
    parameters.coef0 = 0;
    parameters.nu = 0.5;
    parameters.cache_size = 100;
    parameters.C = 1;
    parameters.eps = 1e-3;
    parameters.p = 0.1;
    parameters.shrinking = 1;
    parameters.probability = 0;
    parameters.nr_weight = 0;
    parameters.weight_label = NULL;
    parameters.weight = NULL;


    //get the strings for all of the data sets
    vector<vector<double>> fold1Training;
    vector<vector<double>> fold1Testing;//testing AND validation for the fold
    vector<vector<double>> fold2Training;
    vector<vector<double>> fold2Testing;//testing AND validation for the fold
    vector<vector<double>> fold3Training;
    vector<vector<double>> fold3Testing;//testing AND validation for the fold

    vector<vector<double>> tempVector;

    //load in all of the data
    fold1Training = loadPCA("GenderData/genderdata/48_60/trPCA_01.txt");
    fold2Training = loadPCA("GenderData/genderdata/48_60/trPCA_02.txt");
    fold3Training = loadPCA("GenderData/genderdata/48_60/trPCA_03.txt");

    fold1Testing = loadPCA("GenderData/genderdata/48_60/tsPCA_01.txt");
    fold2Testing = loadPCA("GenderData/genderdata/48_60/tsPCA_02.txt");
    fold3Testing = loadPCA("GenderData/genderdata/48_60/tsPCA_03.txt");

    tempVector = loadPCA("GenderData/genderdata/48_60/valPCA_01.txt");
    fold1Testing.insert(fold1Testing.end(), tempVector.begin(), tempVector.end());
    tempVector = loadPCA("GenderData/genderdata/48_60/valPCA_02.txt");
    fold2Testing.insert(fold2Testing.end(), tempVector.begin(), tempVector.end());
    tempVector = loadPCA("GenderData/genderdata/48_60/valPCA_03.txt");
    fold3Testing.insert(fold3Testing.end(), tempVector.begin(), tempVector.end());


    //load in the classes for each sample, maintain exact same loading order!

//
    //problem
    //y is gender ID, 1 male, 2 female
    //30 coeffs for eigenfaces per training image
        //x[imgcount][30] basically


    prob.l = 4;

    //x values matrix of xor values
    double matrix[prob.l][2];
    matrix[0][0] = 1;
    matrix[0][1] = 1;

    matrix[1][0] = 1;
    matrix[1][1] = 0;

    matrix[2][0] = 0;
    matrix[2][1] = 1;

    matrix[3][0] = 0;
    matrix[3][1] = 0;


    svm_node** x = Malloc(svm_node*,prob.l);

    //Trying to assign from matrix to svm_node training examples
    for (int row = 0;row <prob.l; row++){
        svm_node* x_space = Malloc(svm_node,3);
        for (int col = 0;col < 2;col++){
            x_space[col].index = col;
            x_space[col].value = matrix[row][col];
        }
        x_space[2].index = -1;      //Each row of properties should be terminated with a -1 according to the readme
        x[row] = x_space;
    }

    prob.x = x;

    //yvalues
    prob.y = Malloc(double,prob.l);
    prob.y[0] = -1;
    prob.y[1] = 1;
    prob.y[2] = 1;
    prob.y[3] = -1;

    //Train model---------------------------------------------------------------------
    svm_model *model = svm_train(&prob,&parameters);


    //Test model----------------------------------------------------------------------
    svm_node* testnode = Malloc(svm_node,3);
    testnode[0].index = 0;
    testnode[0].value = 1;
    testnode[1].index = 1;
    testnode[1].value = 0;
    testnode[2].index = -1;

    //This works correctly:
    double retval = svm_predict(model,testnode);
    printf("retval: %f\n",retval);


    svm_destroy_param(&parameters);
    free(prob.y);
    free(prob.x);
    free(x_space);

    return 0;
}
