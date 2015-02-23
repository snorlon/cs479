Requirements:

    Linux machine
    OpenCV
    CMake

How to run:

    From scratch:
        rm CMakeCache.txt
        cmake ../
        Continue to next part

    With current makefile:
        reset; make clean; make
        ./Bayesian <int seed> <Mean A x> <Mean A y> <Mean B x> <Mean B y> <Covariance A 0,0> <Covariance A 0,1> <Covariance A 1,0> <Covariance A 1,1> <Covariance B 0,0> <Covariance B 0,1> <Covariance B 1,0> <Covariance B 1,1> <Class A prior probability> <Class B prior probability>

        Example for 1b:
            ./Bayesian 5 1 1 4 4 1 0 0 1 1 0 0 1 0.3 0.7

Issues: 




Credits:




