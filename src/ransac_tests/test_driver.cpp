#include <iostream>
#include <fstream>
#include <vector>

#include "parse_point_csv.hpp"
#include "segment_points.hpp"

using namespace std;

int main(int argc, char ** argv) {

    Point ** points = parse_points(argv[1]);
    
    vector<int> ids = get_tree_ids(points);

    cout << "total num ids: " << ids.size() << endl;

    Point ** subset_points = subset_points_within_z_range(points, 1.3, 1.34);
    Point ** subset_points_id = subset_points_by_tree_id(subset_points, 13);

    int i = 0;
    Point * current_point;
    do {
        current_point = subset_points_id[i];
        i++; 
        cout << "x: " << current_point->x << " y: " << current_point->y << " z: " << current_point->z << " id: " << current_point->tree_id << endl;
        cout << "iteration: " << i << endl;
    }
    while (subset_points_id[i] != NULL);



    return 0;
}