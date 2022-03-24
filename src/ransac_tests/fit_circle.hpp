#ifndef FIT_CIRCLE_HPP
#define FIT_CIRCLE_HPP

struct Point {
    float x;
    float y;
};


struct Circle {
    float x;
    float y;
    float radius;
};

Circle * find_circle(Point * first, Point * second, Point * third);
int ransac_circle(Circle * circle, Point ** points, int num_points, float range);



#endif