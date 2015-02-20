#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "boxmuller.c"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    int seed = time(NULL);

    if(argc > 1)
    {
        //use the first parameter as a seed for replicating results
        seed = atoi(argv[1]);
    }

    srand(seed);

    for(int i=0; i< 20; i++)
        cout<<ranf()<<endl;

}
