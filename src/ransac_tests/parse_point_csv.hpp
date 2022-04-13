#ifndef PARSE_POINT_CSV_HPP
#define PARSE_POINT_CSV_HPP

using namespace std;


struct Point {
    float x;
    float y;
    float z;
    int tree_id;
};


/**
 * @brief - Parse a csv file of points into an array of Point structs. Assumes
 * csv file has four columns: {x, y, z, id}.
 * 
 * @param filename - path to csv file to be parsed
 * @return Point** - array of pointers to parsed points
 */
vector<Point *> parse_points(char * filename);

/**
 * @brief - Get the next comma separated value from line
 * 
 * @param line - line of comma separated values
 * @param reset - reset the static offset to zero for parsing a new line
 * @return char* - parsed comma separated values
 */
char * get_next_csv(char * line, bool reset);


#endif