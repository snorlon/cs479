#include "../headers/MatrixMath.h"
#include <iostream>

using namespace std;

// Calculates the determinant of a 2x2 matrix and return the value
double determinant(double A[][2]){
	double a, b;
	a = A[0][0] * A[1][1];
	b = A[0][1] * A[1][0];

	return (a-b);
}

// Gets the transpose of a matrix given its size
void transpose(int M, int N, double A[][1], double out[][2]){
	int i,j;
	for(i=0;i<N;i++)
		for(j=0; j<M;j++)
			out[i][j] = A[j][i];
}

// Calculates the inverse of matrix A and outputs it to B
void inverse(double A[][2], double B[][2]){
	float det;
	det = determinant(A);

	B[0][0] = (1/det) * A[1][1];
	B[1][1] = (1/det) * A[0][0];
	B[0][1] = (1/det) * -A[0][1];
	B[1][0] = (1/det) * -A[1][0];
}

// Multiplys nx2 and 2x2 matrices together where it takes in the sizes of A and B, the matrices themselves, and outputs it 
bool multiply2(int aM, int aN, int bM, int bN, double A[][2], double B[][2], double out[][2]){
	int i,j,k;
	if(aN==bM){
		for(i=0;i<aM;i++){
			for(j=0;j<bN;j++){
				out[i][j]=0;
                for(k=0;k<bM;k++){
                    out[i][j] += A[i][k] * B[k][j];
                }
            }
        }
		return true;
	}

	return false;
}

// Multiplys 1x2 and 2x1 matrices together where it takes in the matrices themselves, and outputs it 
void multiply1( double A[][2], double B[][1], double out[][1]){
	int i,j,k;
	for(i=0;i<1;i++){
		for(j=0;j<1;j++){
			out[i][j]=0;
            for(k=0;k<2;k++){
				out[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Adds two 2x2 matrices together
void add(int M, int N, double A[][2], double B[][2], double out[][2]){
	int i, j;
	for(i=0;i<M;i++)
		for(j=0; j<N; j++)
			out[i][j] = A[i][j] + B[i][j];
}

// subtracts two 2x1 matrices
void subtract(int M, int N, double A[][1], double B[][1], double out[][1]){
	int i,j;
		for(i=0;i<M;i++)
		for(j=0; j<N; j++)
			out[i][j] = (A[i][j] - B[i][j]);
}
