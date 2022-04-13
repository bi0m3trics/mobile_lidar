#ifndef SEGMENT_POINTS_HPP
#define SEGMENT_POINTS_HPP

#include <random>
#include <vector>
#include "parse_point_csv.hpp"

using namespace std;

vector<Point *> subset_points_by_tree_id(vector<Point *> points, int tree_id);


vector<Point *> subset_points_within_z_range(vector<Point *> points, float lower, float upper);


vector<int> get_tree_ids(vector<Point *> points);


vector<Point *> get_three_random_points(vector<Point *> points, 
                                                    default_random_engine gen);


vector<Point *> remove_outliers(vector<Point *> points);


vector<float> get_dimension_values(vector<Point *> points, char dimension);


float get_mean(vector<float> values);
float get_std_dev(vector<float> values);
float get_z_score(float value, vector<float> values);

#endif