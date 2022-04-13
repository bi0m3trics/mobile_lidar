#include <iostream>
#include <fstream>
#include <vector>

#include "parse_point_csv.hpp"
#include "segment_points.hpp"
#include "fit_circle.hpp"

using namespace std;

int main(int argc, char ** argv) {

    vector<Point *> points = parse_points(argv[1]);

    // set tree id and iterations
    int id = 1;
    int iterations = 5000;

    // ransac cylinder
    vector<Point *> subset_points_id = subset_points_by_tree_id(points, id);

        // write points to file for visual check
        ofstream out_file;
        out_file.open("points_out.csv");
        out_file << "id,x,y,z" << endl;
        for (int i = 0; i < subset_points_id.size(); i++) {
            out_file << id << "," << subset_points_id[i]->x << "," << subset_points_id[i]->y << "," << subset_points_id[i]->z << endl;
        }

    cout << "num all points: " << subset_points_id.size() << endl;
    vector<Point *> inliers = remove_outliers(subset_points_id);
    cout << "num inliers: " << inliers.size() << endl;

    Cylinder * best_cylinder = perform_ransac(inliers, iterations);


    cout << "best cylinder, x: " << best_cylinder->x << " y: " << best_cylinder->y;
    cout << " z: " << best_cylinder->z << " radius: " << best_cylinder->radius;
    cout << " x angle: " << best_cylinder->x_angle << " y angle: " << best_cylinder->y_angle << endl;
    

    return 0;   
}