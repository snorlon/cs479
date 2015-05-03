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
struct svm_problem prob1;
struct svm_problem prob2;
struct svm_problem prob3;
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
    parameters.cache_size = 100;
    parameters.C = 1; //minimum for maximum correct on first testing set
    parameters.eps = 1e-5;
    parameters.p = 0.1;
    parameters.weight_label = NULL;
    parameters.weight = NULL;

    int currentSet = 1; //1, 2, or 3

    //set gamma and C with parameters
    if(argc >= 3)
    {
        parameters.C = atof(argv[1]);
        parameters.gamma = atof(argv[2]);
    }
    if(argc >= 4)
    {
        currentSet = atoi(argv[3]);
    }
    if(argc >= 5)
    {
        parameters.degree = atoi(argv[4]);
    }

    //get the strings for all of the data sets
    vector<vector<double>> fold1Training;
    vector<vector<double>> fold1Testing;//testing AND validation for the fold
    vector<vector<double>> fold2Training;
    vector<vector<double>> fold2Testing;//testing AND validation for the fold
    vector<vector<double>> fold3Training;
    vector<vector<double>> fold3Testing;//testing AND validation for the fold

    vector<int> fold1TrainingClass = vector<int>();
    vector<int> fold1TestingClass = vector<int>();
    vector<int> fold2TrainingClass = vector<int>();
    vector<int> fold2TestingClass = vector<int>();
    vector<int> fold3TrainingClass = vector<int>();
    vector<int> fold3TestingClass = vector<int>();

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

    ifstream input;
    int tempVal;
    input.open("GenderData/genderdata/48_60/TtrPCA_01.txt");
    input>>tempVal;
    while(input.good())
    {
        fold1TrainingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TtrPCA_02.txt");
    input>>tempVal;
    while(input.good())
    {
        fold2TrainingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TtrPCA_03.txt");
    input>>tempVal;
    while(input.good())
    {
        fold3TrainingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TtsPCA_01.txt");
    input>>tempVal;
    while(input.good())
    {
        fold1TestingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TvalPCA_01.txt");
    input>>tempVal;
    while(input.good())
    {
        fold1TestingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TtsPCA_02.txt");
    input>>tempVal;
    while(input.good())
    {
        fold2TestingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TvalPCA_02.txt");
    input>>tempVal;
    while(input.good())
    {
        fold2TestingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TtsPCA_03.txt");
    input>>tempVal;
    while(input.good())
    {
        fold3TestingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();
    input.open("GenderData/genderdata/48_60/TvalPCA_03.txt");
    input>>tempVal;
    while(input.good())
    {
        fold3TestingClass.push_back(tempVal);

        input>>tempVal;
    }
    input.close();
    input.clear();

    //set the problem parameters
    //we'll only do fold 1 for now
    //problem
    //y is gender ID, 1 male, 2 female
    //30 coeffs for eigenfaces per training image
        //x[imgcount][30] basically

    vector<vector<double>> currentTraining;
    vector<vector<double>> currentTesting;

    vector<int> currentTrainingClass;
    vector<int> currentTestingClass;

    switch(currentSet)
    {
        case 1:
            currentTraining = fold1Training;
            currentTesting = fold1Testing;
            currentTrainingClass = fold1TrainingClass;
            currentTestingClass = fold1TestingClass;
            break;
        case 2:
            currentTraining = fold2Training;
            currentTesting = fold2Testing;
            currentTrainingClass = fold2TrainingClass;
            currentTestingClass = fold2TestingClass;
            break;
        case 3:
            currentTraining = fold3Training;
            currentTesting = fold3Testing;
            currentTrainingClass = fold3TrainingClass;
            currentTestingClass = fold3TestingClass;
            break;
    }

    prob1.l = currentTraining.size();
    svm_node** x = Malloc(svm_node*,prob1.l);
    for (int row = 0; row <prob1.l; row++){
        svm_node* x_space = Malloc(svm_node,31);
        for (int col = 0;col < 30; col++){
            x_space[col].index = col;
            x_space[col].value = currentTraining[row][col];
        }
        x_space[30].index = -1;      //Each row of properties should be terminated with a -1 according to the readme
        x[row] = x_space;
    }

    prob1.x = x;

    prob1.y = Malloc(double,prob1.l);
    for(int i=0; i<currentTrainingClass.size(); i++)
    {
        prob1.y[i] = currentTrainingClass[i];
    }
    //I WANNA BE THE VERY BEST. LIKE NO ONE EVER WAS.
    svm_model *model1 = svm_train(&prob1,&parameters);


cout<<"HERE"<<endl;


    //TESTING

    double correctRate1 = 0;
    double correctRate2 = 0;
    double correctRate3 = 0;


    int testSize = currentTestingClass.size();
    int correctCount = testSize;
    for(int sample = 0; sample < testSize; sample++)
    {
        svm_node* testnode = Malloc(svm_node,31);
        for(int i=0; i<30; i++)
        {
            testnode[i].index = i;
            testnode[i].value = currentTesting[sample][i];
        }
        testnode[30].index = -1;

        //This works correctly:
        double retval = svm_predict(model1,testnode);
        int realVal = currentTestingClass[sample];
        string correctStr = "correct";
        if(retval != realVal)
        {
            correctStr = "WRONG";
            correctCount--;
        }
        //printf("%s    sample: %d    retval: %f    real value: %d\n", correctStr.c_str(), sample, retval ,realVal);
    }
    correctRate1 = (double) 100*correctCount/testSize;

    cout<<correctRate1<<"%"<<endl;


    //x values matrix of xor values
    /*double matrix[prob.l][2];
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
    printf("retval: %f\n",retval);*/


    svm_destroy_param(&parameters);
    free(prob1.y);
    free(prob1.x);
    free(prob2.y);
    free(prob2.x);
    free(prob3.y);
    free(prob3.x);
    free(x_space);

    return 0;
}
