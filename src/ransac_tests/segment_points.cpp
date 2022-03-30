#include <iostream>
#include <vector>
#include <algorithm>

#include "segment_points.hpp"
#include "parse_point_csv.hpp"

using namespace std;


Point ** subset_points_by_tree_id(Point ** points, int tree_id) {

    vector<Point *> * subset_points = new vector<Point *>;
    int i = 0;
    do {
        if (points[i]->tree_id == tree_id) {
cout << "tree id arg" << tree_id << " tree id of point: " << points[i]->tree_id << endl;
            subset_points->push_back(points[i]);
        }
        i++;
    }
    while (points[i] != NULL);
    subset_points->push_back(NULL);

    if (subset_points->size() > 0) return subset_points->data();
    return NULL;
}


Point ** subset_points_within_z_range(Point** points, float lower, float upper) {

    // count number of points within range
    vector<Point *> * subset_points = new vector<Point *>;
    int i = 0;
    do {
        if (points[i]->z >= lower && points[i]->z <= upper) {
            subset_points->push_back(points[i]);
        }
        i++;
    }
    while (points[i] != NULL);
    subset_points->push_back(NULL);

    if (subset_points->size() > 0) return subset_points->data();
    return NULL;
}


vector<int> get_tree_ids(Point ** points) {

    vector<int> tree_ids;

    int id;
    int i = 0;
    do {
        id = points[i]->tree_id;
        
        if ( find( tree_ids.begin(), tree_ids.end(), id ) == tree_ids.end() ) {
            tree_ids.push_back(id);
        }

        i++;
    }
    while (points[i] != NULL);

    return tree_ids;
}