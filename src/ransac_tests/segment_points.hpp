#ifndef SEGMENT_POINTS_HPP
#define SEGMENT_POINTS_HPP

#include "parse_point_csv.hpp"

using namespace std;

vector<Point *> subset_points_by_tree_id(vector<Point *> points, int tree_id);


vector<Point *> subset_points_within_z_range(vector<Point *> points, float lower, float upper);


vector<int> get_tree_ids(vector<Point *> points);


#endif