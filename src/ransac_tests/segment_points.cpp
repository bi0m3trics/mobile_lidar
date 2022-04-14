#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <limits>
#include <math.h>

#include "segment_points.hpp"
#include "parse_point_csv.hpp"

using namespace std;


vector<Point *> subset_points_by_tree_id(vector<Point *> points, int tree_id) {

    vector<Point *> subset_points;
    for (int i = 0; i < points.size(); i++) {
        if (points[i]->tree_id == tree_id) {
            subset_points.push_back(points[i]);
        }
    }
    return subset_points;
}


vector<Point *> subset_points_within_z_range(vector<Point*> points, float lower, float upper) {

    vector<Point *> subset_points;
    for (int i = 0; i < points.size(); i++) {
        if (points[i]->z >= lower && points[i]->z <= upper) {
            subset_points.push_back(points[i]);
        }
    }
    return subset_points;
}


vector<int> get_tree_ids(vector<Point *> points) {

    vector<int> tree_ids;
    int id;
    for (int i = 0; i < points.size(); i++) {
        id = points[i]->tree_id;
        
        if ( find( tree_ids.begin(), tree_ids.end(), id ) == tree_ids.end() ) {
            tree_ids.push_back(id);
        }
    }
    return tree_ids;
}


vector<Point *> get_three_random_points(vector<Point *> points, 
                                                    default_random_engine gen) {

    // subset the points to 10cm slice at breast height
    vector<Point *> subset_points = 
        subset_points_within_z_range(points, 1.3, 1.4);

    // return empty if less than three points in slice
    if (subset_points.size() < 3) {
        vector<Point *> empty;
        return empty;
    }

    // uniform distribution of indexes into subsetted points
    uniform_int_distribution<int> uniform_dist(0, subset_points.size() - 1);

    // randomly select points
    vector<Point *> selected_points;
    for (int i = 0; i < 3; i++) {
        selected_points.push_back( subset_points[uniform_dist(gen)] );
    }
cout << "about to return selected points" << endl;
    return selected_points;
}


vector<Point *> remove_outliers(vector<Point *> points) {
    vector<Point *> inliers;

    vector<float> x_values = get_dimension_values(points, 'x');
    vector<float> y_values = get_dimension_values(points, 'y');
    vector<float> z_values = get_dimension_values(points, 'z');

    float z_score_x, z_score_y, z_score_z;
    float z_cutoff = 2.0;
    for (int i = 0; i < points.size(); i++) {
        // inefficient; recalculates statistics each iteration; fix later
        z_score_x = get_z_score(points[i]->x, x_values);
        z_score_y = get_z_score(points[i]->y, y_values);
        z_score_z = get_z_score(points[i]->z, z_values);
        if (abs(z_score_x) > z_cutoff 
            || abs(z_score_y) > z_cutoff 
            || abs(z_score_z) > z_cutoff) continue;
        else inliers.push_back(points[i]);
    }
    return inliers;
}


vector<float> get_dimension_values(vector<Point *> points, char dimension) {
    vector<float> values;
    for (int i = 0; i < points.size(); i++) {
        if (dimension == 'x') {
            values.push_back(points[i]->x);
        }
        else if (dimension == 'y') {
            values.push_back(points[i]->y);
        }
        else if (dimension == 'z') {
            values.push_back(points[i]->z);
        }
        else {
            cout << "ERROR: expected dimension to be of x, y, z " << endl;
            exit(-1);
        }
    }
    return values;
}

float get_mean(vector<float> values) {
    float sum = accumulate(begin(values), end(values), 0.0);
    return sum / values.size();
}

float get_std_dev(vector<float> values) {
    float mean = get_mean(values);

    vector<float> numerator;
    for (int i = 0; i < values.size(); i++) {
        numerator.push_back( pow( (values[i] - mean), 2) );
    }

    float numerator_sum = accumulate(begin(numerator), end(numerator), 0.0);
    return sqrt( numerator_sum / values.size() );
}

float get_z_score(float value, vector<float> values) {
    float mean = get_mean(values);
    float std_dev = get_std_dev(values);
    return (value - mean) / std_dev;
}