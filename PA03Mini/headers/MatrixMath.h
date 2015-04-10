/* MATRIXMATH.H */
/* Performs matrix operations on a max of 2x2 matrices for the error bound calculations */


// Calculates the determinant of a 2x2 matrix and return the value
double determinant(double A[][2]);

// Gets the transpose of a matrix given its size
void transpose(int M, int N, double A[][1], double out[][2]);

// Calculates the inverse of matrix A and outputs it to B
void inverse(double A[][2],double B[][2]);

// Multiplys nx2 and 2x2 matrices together where it takes in the sizes of A and B, the matrices themselves, and outputs it 
bool multiply2(int aM, int aN, int bM, int bN, double A[][2], double B[][2], double out[][2]);

// Multiplys 1x2 and 2x1 matrices together where it takes in the matrices themselves, and outputs it 
void multiply1( double A[][2], double B[][1], double out[][1]);

// Adds two 2x2 matrices together where it takes in the sizes of A and B(assumed to be the same), the matrices themselves, and outputs it 
void add(int M, int N, double A[][2], double B[][2], double out[][2]);

// subtracts two 2x1 matrices where it takes in the sizes of A and B (assumed to be the same), the matrices themselves, and outputs it 
void subtract(int M, int N, double A[][1], double B[][1], double out[][1]);

