#include <stdio.h>
#include <math.h>
#include <fstream>
#include <vector>
#include <errno.h>
#include <dirent.h>
#include <string>
#include <sstream>
#include <iostream>

#define pi 3.14159265359

using namespace std;

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

double calcProbability(double point[30], double mean[30], double StDev[30][30])
{
    double PrFinal = 1;
    double n[30];    

    for(int i=0; i<30; i++)
    {
        n[i] = (point[i] - mean[i]) / StDev[i][i];
    }

    double Pr[30];

    for(int i=0; i<30; i++)
    {
        Pr[i] = (1/(2*pi)) * exp(-n[i]*n[i] / 2);
    }

    for(int i=0; i<30; i++)
    {
        PrFinal *= Pr[i];
    }

    return PrFinal;
}

int main(int argc, char * argv[])
{
    int currentSet = 1; //1, 2, or 3

    //set currentSet
    if(argc >= 2)
    {
        currentSet = atoi(argv[1]);
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

    double meanMale[30];
    double meanFemale[30];

    int maleCount = 0;
    int femaleCount = 0;

    for(int i=0; i<30; i++)
    {
        meanMale[i] = 0;
        meanFemale[i] = 0;
    }

    //calculate the mean
    for(int i=0; i<currentTrainingClass.size(); i++)
    {
        for(int j=0; j<31; j++)
        {
            if(currentTrainingClass[i] == 1)
            {
                meanMale[j] += currentTraining[i][j];

                maleCount++;
            }
            else
            {
                meanFemale[j] += currentTraining[i][j];

                femaleCount++;
            }
        }
    }

    for(int i=0; i<30; i++)
    {
        meanMale[i] /= maleCount;
        meanFemale[i] /= femaleCount;
    }

    //covariance!
    //male/female might be backwards, unsure, doesn't really matter here!
    double covarianceMale[30][30];
    double covarianceFemale[30][30];

    maleCount = 0;
    femaleCount = 0;

    for(int i=0; i<30; i++)
    {
        for(int j = i; j<30; j++)
        {
            covarianceMale[i][j] = 0;
            covarianceFemale[i][j] = 0;
        }
    }

    for(int k=0; k<currentTrainingClass.size(); k++)
    {
        for(int i=0; i<30; i++)
        {
            for(int j = i; j<30; j++)
            {
                if(currentTrainingClass[k] == 1)
                {
                    double pt1 = currentTraining[k][i] - meanMale[i];
                    double pt2 = currentTraining[k][j] - meanMale[j];

                    double result = pt1*pt2;

                    covarianceMale[i][j] += result;

                    if(j!=i)
                        covarianceMale[j][i] += result;
                }
                else
                {
                    double pt1 = currentTraining[k][i] - meanFemale[i];
                    double pt2 = currentTraining[k][j] - meanFemale[j];

                    double result = pt1*pt2;

                    covarianceFemale[i][j] += result;

                    if(j!=i)
                        covarianceFemale[j][i] += result;
                }
            }
        }
        if(currentTrainingClass[k] == 1)
            maleCount++;
        else
            femaleCount++;
    }

    for(int i=0; i<30; i++)
    {
        for(int j = 0; j<30; j++)
        {
            covarianceMale[i][j] /= maleCount;
            covarianceFemale[i][j] /= femaleCount;

            if(i == j)
            {
                covarianceMale[i][j] = sqrt(covarianceMale[i][j]);
                covarianceFemale[i][j] = sqrt(covarianceFemale[i][j]);
            }
        }
    }

    int correctCount = 0;

    //TESTING
    for(int i=0; i<currentTestingClass.size(); i++)
    {
        double* data = &currentTesting[i][0];

        //calculate chance that the eigen coeffs belong to either gender   
        double pAc = calcProbability(data, meanMale, covarianceMale) * 0.5;
        double pBc = calcProbability(data, meanFemale, covarianceFemale) * 0.5;

        int classification = 1;

        if(pAc < pBc)
            classification = 2;

        if(classification == currentTestingClass[i])
            correctCount++;
    }

    cout<<((double) 100*correctCount / currentTestingClass.size())<<"%"<<endl;


    return 0;
}
