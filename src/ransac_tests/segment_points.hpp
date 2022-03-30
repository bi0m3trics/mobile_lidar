#ifndef SEGMENT_POINTS_HPP
#define SEGMENT_POINTS_HPP

#include "parse_point_csv.hpp"

using namespace std;

Point ** subset_points_by_tree_id(Point ** points, int tree_id);


Point ** subset_points_within_z_range(Point** points, float lower, float upper);


vector<int> get_tree_ids(Point ** points);


#endif