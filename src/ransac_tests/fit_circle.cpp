#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <time.h>
#include <math.h>
#include <limits>

#include "fit_circle.hpp"
#include "segment_points.hpp"

using namespace std;

Cylinder * perform_ransac(vector<Point *> points, int iterations) {

    // prep the normal distribution

        // create the random number generator
        default_random_engine gen;
         
        // normal distribution to model x and y angles of cylinder
        normal_distribution<float> normal_dist(0, 10);

    // ---

    // RANSAC initial circle
    double best_squared_sum = numeric_limits<double>::max();
    double current_squared_sum;
    Circle * current_circle;
    Circle * best_circle = new Circle;
    for (int i = 0; i < iterations; i++) {

        // seed the random generator
        gen.seed( time(NULL) + i );

        // find a circle through three random points  
        vector<Point *> three_points = get_three_random_points(points, gen);

            // if no points at breast height then cylinder fitting not meaningful
            if (three_points.empty()) {
                return new Cylinder {0, 0, 0, 0, 0, 0};
            }

        current_circle = find_circle(three_points);

            // skip iteration if circle finding was unsuccessful
            if (current_circle == NULL) {
                continue;
            }

        // ransac the circle
        current_squared_sum = ransac_circle(points, current_circle);

        // keep only best circle
        if (current_squared_sum < best_squared_sum) {
            best_squared_sum = current_squared_sum;
            best_circle = current_circle;

        }
        else {
            delete current_circle;
        }
    }
    
    // RANSAC cylinder
    best_squared_sum = numeric_limits<double>::max();
    Cylinder * best_cylinder = new Cylinder;
    for (int i = 0; i < iterations; i++) {

        // seed the random generator
        gen.seed( time(NULL) + i );
        
        // create cylinder using generated circle, a random angle in the x 
        // dimension, and a random angle in the y dimension 
        Cylinder * current_cylinder = new Cylinder {
            .x = best_circle->x,
            .y = best_circle->y,
            .z = best_circle->z,
            .radius = best_circle->radius,
            .x_angle = normal_dist(gen),
            .y_angle = normal_dist(gen)
        };

        // ransac the cylinder
        current_squared_sum = ransac_cylinder(points, current_cylinder);

        // update the best result if current result is better
        if (current_squared_sum < best_squared_sum) {
            best_squared_sum = current_squared_sum;
            best_cylinder = current_cylinder;
        }
        else {
            delete current_cylinder;
        }

    }

    cout << "best distances squared sum: " << best_squared_sum << endl;
    cout << "best circle, x: " << best_circle->x << " y: " << best_circle->y;
    cout << " z: " << best_circle->z << " radius: " << best_circle->radius << endl;

    return best_cylinder;
}


double ransac_circle(vector<Point *> points, Circle * circle) {

    // subset the points to 10cm slice at breast height
    vector<Point *> subset_points = 
        subset_points_within_z_range(points, 1.3, 1.4);
cout << "num points in z slice circle fit: " << subset_points.size() << endl;

        /*
        ofstream out_file;
        out_file.open("points_out_slice.csv");
        out_file << "x,y,z" << endl;
        for (int i = 0; i < subset_points.size(); i++) {
            out_file << subset_points[i]->x << "," << subset_points[i]->y << "," << subset_points[i]->z << endl;
        }
        out_file.close();
        */

    // perform ransac
    float distance;
    double distances_squared_sum = 0;
    for (int i = 0; i < subset_points.size(); i++) {
        float x_distance = circle->x - subset_points[i]->x;
        float y_distance = circle->y - subset_points[i]->y;

        // distance from center of circle
        distance =  sqrt( pow(x_distance, 2) + pow(y_distance, 2) );

        // distance from circle 
        distance = distance - circle->radius;

        distances_squared_sum += pow(distance, 2);
    }
    return distances_squared_sum;
}


double ransac_cylinder(vector<Point *> points, Cylinder * cylinder) {
    float distance;
    double distances_squared_sum = 0;
    float x_offset, y_offset, x_distance, y_distance;
    for (int i = 0; i < points.size(); i++) {

        // find offset from center rod given angle
        x_offset = offset_from_cylinder_center(points[i], cylinder, 'x');
        y_offset = offset_from_cylinder_center(points[i], cylinder, 'y');
   
        // find x and y distance from cylinder surface, using the x and y
        // offsets (which take into account cylinder lean)
        x_distance = cylinder->x - x_offset - points[i]->x;
        y_distance = cylinder->y - y_offset - points[i]->y;

        // distance from center rod
        distance =  sqrt( pow(x_distance, 2) + pow(y_distance, 2) );

        // distance from cylinder surface
        distance = distance - cylinder->radius;

        distances_squared_sum += pow(distance, 2);
    }

    return distances_squared_sum;
}


float offset_from_cylinder_center(Point * point, Cylinder * cylinder, 
                                                               char dimension) {
    float z_difference = point->z - cylinder->z;
    switch (dimension) {
        case 'x':
            if (cylinder->x_angle < 0) {
                return z_difference / tan( (-90 - cylinder->x_angle) * M_PI/180 );
            }
            return z_difference / tan( (90 - cylinder->x_angle) * M_PI/180 );
        case 'y':
            if (cylinder->y_angle < 0) {
                return z_difference / tan( (-90 - cylinder->y_angle) * M_PI/180 );
            }
            return z_difference / tan( (90 - cylinder->y_angle) * M_PI/180 );
        default:
            cout << "ERROR: expected x or y dimension for offset" << endl;
            exit(-1);
    }
}


Circle * find_circle(vector<Point *> points) {

    Point * first = points[0];
    Point * second = points[1];
    Point * third = points[2];

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

    // find average z value
    float average_z = (first->z + second->z + third->z) / 3;

    // construct and return circle
    Circle * circle = new Circle {
        .x = intersection.x,
        .y = intersection.y,
        .z = average_z,
        .radius = radius,
    };
    return circle;
}

