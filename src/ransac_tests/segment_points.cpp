#include <iostream>
#include <vector>
#include <algorithm>

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