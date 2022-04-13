#ifndef FIT_CIRCLE_HPP
#define FIT_CIRCLE_HPP

#include "parse_point_csv.hpp"

using namespace std;

struct Circle {
    float x;
    float y;
    float z;
    float radius;
};

struct Cylinder {
    float x;
    float y;
    float z;
    float radius;
    float x_angle;
    float y_angle;
};

Cylinder * perform_ransac(vector<Point *> points, int iterations);

double ransac_circle(vector<Point *> points, Circle * circle);

double ransac_cylinder(vector<Point *> points, Cylinder * cylinder);

float offset_from_cylinder_center(Point * point, Cylinder * cylinder, 
                                                               char dimension);

Circle * find_circle(vector<Point *> points);



#endif