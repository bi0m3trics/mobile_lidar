#include <iostream>
#include <fstream>
#include <random>
#include <time.h>
#include <math.h>

using namespace std;

int main(int argc, char** argv) {

    if (argc != 2) {
        cout << "Expected number of points as argument" << endl;
        exit(-1);
    }

    int radius = 1;
    double x, y;

    // create the random number generator and seed it
    default_random_engine gen;
    gen.seed( time(NULL) );

    // set up the binomial and uniform random distributions
    binomial_distribution<int> bi_dist(100, 0.5);
    uniform_real_distribution<double> uniform_dist(0 - radius, 0 + radius);

    ofstream points_file;
    points_file.open("generated_points.csv");
    points_file << "x,y" << endl;

    int i;
    int num_points = atoi(argv[1]);
    for (i = 0; i < num_points; i++) {

        // randomly generate the x coordinate
        x = uniform_dist(gen);

        // solve for y
        y = sqrt( abs( pow(radius, 2) - pow(x, 2) ) );

        // randomly determine whether y should be negative
        if (uniform_dist(gen) < 0) {
            y = -y;
        }

        // simulate noise to y using binomial distribution
        y = y + ( (bi_dist(gen) / 100.0 ) - 0.5 );

        // simulate noise to x using binomial distribution
        x = x + ( (bi_dist(gen) / 100.0 ) - 0.5 );

        // write the point to a file
        points_file << x << "," << y << endl;
        
    }

    points_file.close();

    return 0;

}