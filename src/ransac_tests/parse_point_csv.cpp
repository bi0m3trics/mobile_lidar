#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

#include "parse_point_csv.hpp"

using namespace std;


vector<Point *> parse_points(char * filename) {

    // open csv file
    ifstream points_csv;
    points_csv.open(filename);

    vector<char *> points_as_strings;

    // parse points into vector
    char * line = new char[100];
    points_csv >> line; // skip the header
    while( points_csv >> line ) {
        char * temp_str = new char[100];
        for (int i = 0; i < 100; i++) {
            temp_str[i] = line[i];
        }
        points_as_strings.push_back(temp_str);
    }

    vector<Point *> points;

    // parse csv strings into array of point structs
    for (int i = 0; i < points_as_strings.size(); i++) {
        Point * point = new Point {
            .x = stof( get_next_csv(points_as_strings[i], true) ),
            .y = stof( get_next_csv(points_as_strings[i], false) ),
            .z = stof( get_next_csv(points_as_strings[i], false) ),
            .tree_id = stoi( get_next_csv(points_as_strings[i], false) )
        };

        points.push_back(point);

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


