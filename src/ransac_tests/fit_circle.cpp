#include <iostream>
#include <fstream>
#include <random>
#include <time.h>
#include <math.h>

#include "fit_circle.hpp"

using namespace std;

int main(int argc, char ** argv) {

    if (argc != 2) {
        cout << "Expecting ransac iterations as command line arg." << endl;
        exit(-1);
    }

    ifstream points_file;
    points_file.open("generated_points.csv");

    // count number of points
    int line_counter = 0;
    char line[30];
    while(points_file >> line) {
        line_counter++;
    }
    int num_points = line_counter - 1; // header line is not a point
    
    // reopen the file
    points_file.close();
    points_file.open("generated_points.csv");

    // parse points into array
    char points_as_strings[num_points][30];

        // initialize to all '\0' for proper termination
        for (int i = 0; i < num_points; i++) {
            for (int j=0; j < 30; j++) {
                points_as_strings[i][j] = '\0';
            }
        }

    points_file >> line; // skip the header
    line_counter = 0;
    while(points_file >> points_as_strings[line_counter]) {
        line_counter++;
    }

    // convert to array of points
    Point * points[num_points];
    char temp_str_x[15];
    char temp_str_y[15];
    for (int i = 0; i < num_points; i++) {
   
        // parse csv values of each point
        int j = 0;
        while (points_as_strings[i][j] != ',') {
            temp_str_x[j] = points_as_strings[i][j];
            j++;
        }
        temp_str_x[j] = '\0';

        j++; // move past ","
        int k = 0;
        while (points_as_strings[i][j] != '\0') {
            temp_str_y[k] = points_as_strings[i][j];
            k++;
            j++;
        }
        temp_str_y[k] = '\0';

        // create new point struct and add to array
        Point * current_point = new Point;
        current_point->x = stof(temp_str_x);
        current_point->y = stof(temp_str_y);
        
        points[i] = current_point;
    }

    // create the random number generator and seed it
    default_random_engine gen;
    gen.seed( time(NULL) ); 

    // create a uniform distribution 
    uniform_int_distribution<int> uniform_dist(0, num_points);

    // RANSAC
    int ransac_iterations = atoi(argv[1]);
    int best_count = 0;
    Circle * best_circle;
    for (int i = 0; i < ransac_iterations; i++) {

        // randomly select three points from the points array
        Point * selected_points[3];
        for (int i=0; i < 3; i++) {
            int random_index = uniform_dist(gen);
            selected_points[i] = points[random_index];
        }
        
        // find the circle through the three points
        Circle * current_circle;
        current_circle = find_circle(selected_points[0], 
                                     selected_points[1],
                                     selected_points[2]);

            // skip iteration if circle finding was unsuccessful
            if (current_circle == NULL) {
                continue;
            }
        
        // perform ransac with the circle and all points
        int current_count = ransac_circle(current_circle, points, num_points, 0.01);

        // update the best result if current result is better
        if (current_count > best_count) {
            best_count = current_count;
            best_circle = current_circle;
        }

    }

    cout << "best overall count: " << best_count << endl;
    cout << "best circle: " << best_circle->x << ", " << best_circle->y << ", r: " << best_circle->radius << endl;
    
    return 0;
}


int ransac_circle(Circle * circle, Point ** points, int num_points, float offset) {

    float distance;
    int in_range_count = 0;
    for (int i = 0; i < num_points; i++) {
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
            cout << "Division by zero" << endl;
            return NULL;
        }

    first_slope = (first->y - second->y) / (first->x - second->x);
    second_slope = (first->y - third->y) / (first->x - third->x);
    third_slope = (second->y - third->y) / (second->x - third->x);

    // check for colinearity
    if (first_slope == second_slope && second_slope == third_slope) {
        cout << "Points colinear" << endl;
        return NULL;
    }

    // ensure no zero slopes
    if (abs(first_slope) == 0 || abs(second_slope) == 0) {
        cout << "Division by zero" << endl;
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

