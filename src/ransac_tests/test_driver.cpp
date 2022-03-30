#include <iostream>
#include <fstream>
#include <vector>

#include "parse_point_csv.hpp"
#include "segment_points.hpp"
#include "fit_circle.hpp"

using namespace std;

int main(int argc, char ** argv) {

    vector<Point *> points = parse_points(argv[1]);
    
    vector<int> ids = get_tree_ids(points);

    vector<Point *> subset_points_z = subset_points_within_z_range(points, 1.3, 1.4);
    vector<Point *> subset_points_z_id = subset_points_by_tree_id(subset_points_z, 1);

/*
    // get points in z range of some id (set above)
    Point * current_point;
    for (int i = 0; i < subset_points_z_id.size(); i++) {
        current_point = subset_points_z_id[i];
        cout << "x: " << current_point->x << " y: " << current_point->y << " z: " << current_point->z << " id: " << current_point->tree_id << endl;
        cout << "iteration: " << i << endl;
    }
*/

    // ransac 
    perform_ransac(subset_points_z_id, 1000);

    return 0;
}