#include "../headers/MatrixMath.h"
#include <math.h>
#include <iostream>

using namespace std;

// Generate a set of 1000 points for the chernoff bound to be plotted on beta=0 to 1.0 where
// "bound" is the set of points generated, m1 and m2 are the means, and c1 and c2 are
// the covariance matrixes
void makeChernoff(double chernoffBoundPoints[][2], double meanA[][1], double meanB[][1], double covarianceA[][2], double covarianceB[][2])
{
	int i,j,x;
	double trans[1][2];
	double temp1[1][2];
	double temp2[1][1];
	double mTemp[2][1];
	double cTemp[2][2];
	double cTemp1[2][2];
	double cTemp2[2][2];
	double cInv[2][2];
	double det1, det2, detTop, lnNum=0, beta, betaN, coef, kB;

	// calculating 
	for(x =0; x < 10000; x++){
		// set 1-B and B
		beta = x/10000.0f;
		betaN = 1-beta;

		// do matrix math first
		subtract(2,1,meanB,meanA,mTemp);
		transpose(2,1,mTemp,trans);

		for(i=0;i<2;i++)
			for(j=0;j<2;j++)
			cTemp1[i][j] = beta*covarianceA[i][j];

		for(i=0;i<2;i++)
			for(j=0;j<2;j++)
			cTemp2[i][j] = betaN*covarianceB[i][j];

		add(2,2,cTemp1,cTemp2,cTemp);
		inverse(cTemp,cInv);

		multiply2(1,2,2,2,trans,cInv,temp1);
		multiply1(temp1, mTemp, temp2);

		//calculating the last "ln" term
		det1 = determinant(covarianceA);
		det1 = pow(det1, beta);
		det2 = determinant(covarianceB);
		det2 = pow(det2, betaN);
		detTop = determinant(cTemp);
		lnNum = 0.5 * log(detTop/(det1*det2));
		
		// calculate coefficient
		coef = (beta*betaN)/2;
		
		//get k(b)
		chernoffBoundPoints[x][0] = beta;
		kB = (coef*temp2[0][0])+lnNum;
		chernoffBoundPoints [x][1] = exp(-kB);
	}
}

// Calculates the minimum beta value, "minBeta" and gets the Y-value, 'y', for the chernoff bound
// takes in the bound points as a parameter
void findMinB(double bound[][2], double& minBeta, double& y){
	int i;
	double minB=bound[0][0];
	double minY=bound[0][1];

	for(int i=0; i<10000; i++){
		if(bound[i][1] < minY){
			minB = bound[i][0];
			minY = bound[i][1];
		}
	}
	minBeta = minB;
	y = minY;
}


// Calculates the k(beta) for the battacharayya bound where beta = 0.5 and returns it
// takes in the means and the covariances
double getBattacharayyaBound(double m1[][1], double m2[][1], double c1[][2], double c2[][2]){
	int i, j;
	double trans[1][2];
	double temp1[1][2];
	double temp2[1][1];
	double mTemp[2][1];
	double cTemp[2][2];
	double cInv[2][2];
	double det1, det2, detTop, lnNum=0;

	// do matrix math first
	subtract(2,1,m2,m1,mTemp);
	transpose(2,1,mTemp,trans);
	add(2,2,c1,c2,cTemp);
	for(i=0;i<2;i++)
		for(j=0;j<2;j++)
			cTemp[i][j] /= 2;

	inverse(cTemp,cInv);

	multiply2(1,2,2,2,trans,cInv,temp1);
	multiply1(temp1, mTemp, temp2);

	//calculating the last "ln" term
	det1 = determinant(c1);
	det2 = determinant(c2);
	detTop = determinant(cTemp);
	lnNum = 0.5 * log(detTop/(sqrt(det1*det2)));

	return (0.125 * temp2[0][0])+lnNum;
}

// Calculates the probability of error for the battacharayya bound 
//by taking in the priors and the k(b)
double getProb(double pw1, double pw2, double kB){
	return (sqrt(pw1*pw2))*exp(-kB);
}
