Requirements:

    Linux machine
    OpenCV
    CMake

How to run:

    From scratch:
        cmake CMakeLists.txt
        Continue to next part

    With current makefile:
        reset; make clean; make
        ./Bayesian <Mean A x> <Mean A y> <Mean B x> <Mean B y> <Covariance A 0,0> <Covariance A 0,1> <Covariance A 1,0> <Covariance A 1,1> <Covariance B 0,0> <Covariance B 0,1> <Covariance B 1,0> <Covariance B 1,1>

        Example for 1b:
            ./Bayesian 1 1 4 4 1 0 0 1 1 0 0 1

Issues: 
    None known



Code Credits:
    Everett F. Carter Jr:   1D box-muller source
    Chad Brown:             2D box-muller based on 1D, c++ rendering, finding the mean and covariance of the data
    Sarah Koh:              Matrix math
