/* boxmuller.c           Implements the Polar form of the Box-Muller
                         Transformation

                      (c) Copyright 1994, Everett F. Carter Jr.
                          Permission is granted by the author to use
			  this software for any application provided this
			  copyright notice is preserved.

*/

#include <math.h>


double ranf()
{
    double f = (double)(rand() % 100000)/100000.0f;
    return f;
}

double box_muller(double m, double s)	/* normal random variate generator */
{				        /* mean m, standard deviation s */
	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0 * ranf() - 1.0;
			x2 = 2.0 * ranf() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( m + y1 * s );
}


void box_muller2d(double point[2], double m[2], double s[2][2])	/* normal random variate generator */
{				        /* mean m, standard deviation s */
    static double y2[2];
    static int use_last[2] = {0,0};
	
    for(int i=0; i<2; i++)
    {
        double x1, x2, w, y1;

	    if (use_last[i])		        /* use value from previous call */
	    {
		    y1 = y2[i];
		    use_last[i] = 0;
	    }
	    else
	    {
		    do {
			    x1 = 2.0 * ranf() - 1.0;
			    x2 = 2.0 * ranf() - 1.0;
			    w = x1 * x1 + x2 * x2;
		    } while ( w >= 1.0 );

		    w = sqrt( (-2.0 * log( w ) ) / w );
		    y1 = x1 * w;
		    y2[i] = x2 * w;
		    use_last[i] = 1;
	    }

        point[i] = m[i] + y1 * s[i][i];
    }
}


