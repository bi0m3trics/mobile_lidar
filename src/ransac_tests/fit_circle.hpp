#ifndef FIT_CIRCLE_HPP
#define FIT_CIRCLE_HPP

#include "parse_point_csv.hpp"

using namespace std;

struct Circle {
    float x;
    float y;
    float radius;
};

int perform_ransac(vector<Point *> points, int iterations);
Circle * find_circle(Point * first, Point * second, Point * third);
int ransac_circle(Circle * circle, vector <Point *> points, float offset);



#endif