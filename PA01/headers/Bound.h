/* BOUND.H */
/* Performs operations dealing with the chernoff bound and battacharayya bound */


// Generate a set of 100 points for the chernoff bound to be plotted on beta=0 to 1.0 where
// "chernoffBoundPoints" is the set of chernoff points generated, meanA and meanB are the means of class A and class B respectively, and covarianceA and covarianceB are
// the covariance matrixes
void makeChernoff(double chernoffBoundPoints[][2], double meanA[][1], double meanB[][1], double covarianceA[][2], double covarianceB[][2]);

// Calculates the minimum beta value, "minBeta" and gets the Y-value, 'minY', for the chernoff bound
// takes in the chernoff bound points as a parameter
void findMinB(double chernoffBoundPoints[][2], double& minBeta, double& minY);


// Calculates the k(beta) for the battacharayya bound where beta = 0.5 and returns it
// takes in the means and the covariances
double getBattacharayyaBound(double meanA[][1], double meanB[][1], double covarianceA[][2], double covarianceB[][2]);

// Calculates the probability of error for the battacharayya bound 
//by taking in the priors and the k(b) from getBattacharayyaBound
double getProb(double priorProbA, double priorProbB, double kB);


