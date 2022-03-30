#include <iostream>
#include <fstream>
#include <random>
#include <time.h>
#include <math.h>

#include "fit_circle.hpp"

using namespace std;

int perform_ransac(vector<Point *> points, int iterations) {

    // create the random number generator and seed it
    default_random_engine gen;
    gen.seed( time(NULL) ); 

    // create a uniform distribution 
    uniform_int_distribution<int> uniform_dist(0, points.size() - 1);

    // RANSAC
    int ransac_iterations = iterations;
    int best_count = 0;
    Circle * current_circle = NULL;
    Circle * best_circle = NULL;
    for (int i = 0; i < ransac_iterations; i++) {

        // randomly select three points from the points array
        Point * selected_points[3];
        for (int j=0; j < 3; j++) {
            int random_index = uniform_dist(gen);
            selected_points[j] = points[random_index];
        }

        // find the circle through the three points    
        current_circle = find_circle(selected_points[0], 
                                     selected_points[1],
                                     selected_points[2]);

            // skip iteration if circle finding was unsuccessful
            if (current_circle == NULL) {
                continue;
            }
        
        // perform ransac with the circle and all points
        int current_count = ransac_circle(current_circle, points, 0.01);

        // update the best result if current result is better
        if (current_count > best_count) {
            best_count = current_count;
            best_circle = current_circle;
        }
        else {
            delete current_circle;
        }

    }

    cout << "best overall count: " << best_count << endl;
    cout << "best circle: " << best_circle->x << ", " << best_circle->y << ", r: " << best_circle->radius << endl;
    
    return 0;
}


int ransac_circle(Circle * circle, vector <Point *> points, float offset) {

    float distance;
    int in_range_count = 0;
    for (int i = 0; i < points.size(); i++) {
        distance =  sqrt( pow(circle->x - points[i]->x, 2) + 
                          pow(circle->y - points[i]->y, 2) );

        if (distance > circle->radius - offset && 
            distance < circle->radius + offset) {
                in_range_count++;
            }
    }

    return in_range_count;
}


Circle * find_circle(Point * first, Point * second, Point * third) {

    // find slopes of the three lines
    float first_slope, second_slope, third_slope;

        // ensure no division by zero
        if (first->x - second->x == 0 || first->x - third->x == 0 ||
            second->x - third->x == 0 ) {
            return NULL;
        }

    first_slope = (first->y - second->y) / (first->x - second->x);
    second_slope = (first->y - third->y) / (first->x - third->x);
    third_slope = (second->y - third->y) / (second->x - third->x);

    // check for colinearity
    if (first_slope == second_slope && second_slope == third_slope) {
        return NULL;
    }

    // ensure no zero slopes
    if (abs(first_slope) == 0 || abs(second_slope) == 0) {
        return NULL;
    }

    // find midpoints of the two lines
    Point first_mid, second_mid;
    first_mid.x = (first->x + second->x) / 2;
    first_mid.y = (first->y + second->y) / 2;
    second_mid.x = (first->x + third->x) / 2;
    second_mid.y = (first->y + third->y) / 2;
    // find intersection of two perpendicular lines 
    Point intersection;

    float first_perp_slope = -1/first_slope;
    float second_perp_slope = -1/second_slope;

    float first_perp_intercept = first_mid.y - (first_perp_slope * first_mid.x);
    float second_perp_intercept = second_mid.y - (second_perp_slope * second_mid.x);

        // solve for x
        intersection.x = (second_perp_intercept - first_perp_intercept) / 
                         (first_perp_slope - second_perp_slope);

        // solve for y
        intersection.y = first_perp_slope * intersection.x + first_perp_intercept;
    // find radius of circle
    float radius = sqrt( pow(intersection.x - first->x, 2) + 
                         pow(intersection.y - first->y, 2) );

    // construct and return circle
    Circle * circle = new Circle;
    circle->x = intersection.x;
    circle->y = intersection.y;
    circle->radius = radius;

    return circle;
}

