#include "../headers/MatrixMath.h"
#include <math.h>
#include <iostream>

using namespace std;

// Generate a set of 1000 points for the error bound to be plotted on beta=0 to 1.0 where
// "bound" is the set of points generated, m1 and m2 are the means, and c1 and c2 are
// the covariance matrixes
void makeBound(double chernoffBoundPoints[][2], double meanA[][1], double meanB[][1], double covarianceA[][2], double covarianceB[][2])
{
	int i,j,k;
	double trans[1][2];
	double temp1[1][2];
	double temp2[1][1];
	double mTemp[2][1];
	double cTemp[2][2];
	double cTemp1[2][2];
	double cTemp2[2][2];
	double cInv[2][2];
	double determinant1, determinant2, determinantTop, lnNum=0, beta, betaN, coef, kB;

	//generate a point for each increment up to 10000 
	for(k =0; k < 10000; k++)
    {
		// set 1-B and B
		beta = k/10000.0f;//it should be a ratio between 0 and
		betaN = 1-beta;//reverse it

		// do matrix math first
		subtract(2,1,meanB,meanA,mTemp);//store the difference between the two means in a temporary matrix
		transpose(2,1,mTemp,trans);//get the transpose of the result and store it in trans

        //calculate the result of multiplying the beta and the covariance for both classes
		for(i=0;i<2;i++)
			for(j=0;j<2;j++)
    			cTemp1[i][j] = beta*covarianceA[i][j];

		for(i=0;i<2;i++)
			for(j=0;j<2;j++)
    			cTemp2[i][j] = betaN*covarianceB[i][j];

        //add them together and store them in a temporary matrix
		add(2,2,cTemp1,cTemp2,cTemp);
		inverse(cTemp,cInv);//store the inverse of that temporary matric

        //store the result of that transpose and inverse temporaries from before in temp1
		multiply2(1,2,2,2,trans,cInv,temp1);
		multiply1(temp1, mTemp, temp2);//multiply it by the difference of the two means

		//calculating the last "ln" term
		determinant1 = determinant(covarianceA);
		determinant1 = pow(determinant1, beta);//store the determinant of the covariance A matrix to the beta'th power
		determinant2 = determinant(covarianceB);
		determinant2 = pow(determinant2, betaN);//store the determinant of the covariance B matrix to the reversed beta'th power
		determinantTop = determinant(cTemp);
		lnNum = 0.5 * log(determinantTop/(determinant1*determinant2));
		
		// calculate coefficient
		coef = (beta*betaN)/2;
		
		//get k(Beta)
		chernoffBoundPoints[k][0] = beta;
		kB = (coef*temp2[0][0])+lnNum;
		chernoffBoundPoints [k][1] = exp(-kB);
	}
}

// Calculates the minimum beta value, "minBeta" and gets the Y-value, 'y', for the chernoff bound
// takes in the bound points as a parameter
void findMinB(double chernoffBoundPoints[][2], double& minBeta, double& minYOutput)
{
	int i;
	double minB=chernoffBoundPoints[0][0];
	double minY=chernoffBoundPoints[0][1];

    //check each point to see if it is a lower value than the prior minimum
	for(int i=0; i<10000; i++){
		if(chernoffBoundPoints[i][1] < minY){
			minB = chernoffBoundPoints[i][0];
			minY = chernoffBoundPoints[i][1];
		}
	}

	minBeta = minB;
	minYOutput = minY;
}


// Calculates the k(beta) for the battacharayya bound where beta = 0.5 and returns it
// takes in the means and the covariances
double getBattacharayyaBound(double meanA[][1], double meanB[][1], double covarianceA[][2], double covarianceB[][2])
{
	int i, j;
	double trans[1][2];
	double temp1[1][2];
	double temp2[1][1];
	double mTemp[2][1];
	double cTemp[2][2];
	double cInv[2][2];
	double det1, det2, detTop, lnNum=0;

	// do matrix math first
	subtract(2,1,meanB,meanA,mTemp);//store the difference between the means
	transpose(2,1,mTemp,trans);//store the transpose of the difference in "trans"
	add(2,2,covarianceA,covarianceB,cTemp);//store the sum of the covariances

    //halve the covariances
	for(i=0;i<2;i++)
		for(j=0;j<2;j++)
			cTemp[i][j] /= 2;

	inverse(cTemp,cInv);//store their inverse

	multiply2(1,2,2,2,trans,cInv,temp1);//store the product of the transpose of mean differences and the inverse of the covariances
	multiply1(temp1, mTemp, temp2);//and then multiply that by the difference of the means

	//calculating the last "ln" term
	det1 = determinant(covarianceA);
	det2 = determinant(covarianceB);
	detTop = determinant(cTemp);
	lnNum = 0.5 * log(detTop/(sqrt(det1*det2)));

	return (0.125 * temp2[0][0])+lnNum;
}

// Calculates the probability of error for the battacharayya bound 
//by taking in the priors and the e^-k(b)
double getProb(double priorProbA, double priorProbB, double ekB)
{
	return (sqrt((priorProbA*priorProbB)*ekB));
}

// Generates points, in "outputGraph", that can be plotted to show both the battacharyya and Chernoff bounds 
//takes in the plot points "chernoffboundPoints", the minimuzing beta for the chernoff with the corresponding y value,
// and the e^(-k(b)) for the battacharyya
// To be used for an output file that will be plotted in Excel
void makePlot(double outputGraph[][6], double chernoffBoundPoints[][2], double minBeta, double minY, double eKB)
{
	int i;
	double currentChernoffX = 0.0f;
	double currentChernoffY = minY;
	double currentBattaX = 0.0f;
	double currentBattaY = eKB;

	// setting plot points for curve
	for(i=0; i < 10000; i++){
		outputGraph[i][0] = chernoffBoundPoints[i][0];
		outputGraph[i][1] = chernoffBoundPoints[i][1];
	}

	i=0;
	//setting plot points for chernoff horrizontally
	while(currentChernoffX < minBeta){
		outputGraph[i][2] = currentChernoffX;
		outputGraph[i][3] = currentChernoffY;
		currentChernoffX += 0.001;
		i++;
	}
	//setting plot points for chernoff vertically 
	while(currentChernoffY > 0){
		outputGraph[i][2] = currentChernoffX;
		outputGraph[i][3] = currentChernoffY;
		currentChernoffY -= 0.001;
		i++;
	}
	// fill rest with zeros to avoid junk
	while (i <10000){
		outputGraph[i][2] = 0;
		outputGraph[i][3] = 0;
		i++;
	}

	i=0;
	//setting plot points for bhattacharyya horrizontally
	while(currentBattaX < 0.5){
		outputGraph[i][4] = currentBattaX;
		outputGraph[i][5] = currentBattaY;
		currentBattaX += 0.001;
		i++;
	}

	//setting plot points for bhattacharyya vertically 
	while(currentBattaY > 0){
		outputGraph[i][4] = currentBattaX;
		outputGraph[i][5] = currentBattaY;
		currentBattaY -= 0.001;
		i++;
	}
	// fill rest with zeros to avoid junk
	while (i <10000){
		outputGraph[i][4] = 0;
		outputGraph[i][5] = 0;
		i++;
	}

}
