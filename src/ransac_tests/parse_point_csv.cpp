#include <iostream>
#include <fstream>

#include "parse_point_csv.hpp"

using namespace std;


Point ** parse_points(char * filename) {

    // open csv file
    ifstream points_csv;
    points_csv.open(filename);

    // count number of points in file
    int line_counter = 0;
    char line[100];
    while(points_csv >> line) {
        line_counter++;
    }
    int num_points = line_counter - 1; // header line is not a point

    // reopen csv file
    points_csv.close();
    points_csv.open(filename);

    // allocate array and NULL terminate
    Point ** points = new Point *[num_points + 1];
    points[num_points] = NULL;

    // parse points into array
    char points_as_strings[num_points][100];
    points_csv >> line; // skip the header
    line_counter = 0;
    while(points_csv >> points_as_strings[line_counter]) {
        line_counter++;
    }

    // parse csv strings into array of point structs
    for (int i = 0; i < num_points; i++) {
        Point * point = new Point {
            .x = stof( get_next_csv(points_as_strings[i], true) ),
            .y = stof( get_next_csv(points_as_strings[i], false) ),
            .z = stof( get_next_csv(points_as_strings[i], false) ),
            .tree_id = stoi( get_next_csv(points_as_strings[i], false) )
        };

        points[i] = point;

    }

    return points;
}


char * get_next_csv(char * line, bool reset) {
    static int offset = 0;

    // reset offset if desired
    if (reset) {
        offset = 0;
    }

    // move past ',' if necessary
    if (line[offset] == ',') {
            offset++;
    }

    // allocate space for current csv
    int precision = 10;
    char * csv = new char[precision];

    // parse until next comma or EOL
    int i = 0;
    while (line[offset] != ',' && line[offset] != '\0') {

        // discard undesired precision
        if (i >= precision - 1) {
            offset++;
            continue;
        }

        csv[i] = line[offset];
        offset++;
        i++;
    }
    csv[i] = '\0';

    return csv;
}   


