//circle_fit.cpp
#include <Rcpp.h>
using namespace Rcpp;

//' @export
// [[Rcpp::export]]
NumericVector rcpp_circle_fit(NumericMatrix sample){

  /*
   * this uses the bisecting cord method to find the center of the circle
   */

  // find coordinate differences on lines drawn between points
  double line_1_y_diff = sample( 1, 1) - sample( 0, 1);
  double line_1_x_diff = sample( 1, 0) - sample( 0, 0);
  double line_2_y_diff = sample( 2, 1) - sample( 1, 1);
  double line_2_x_diff = sample( 2, 0) - sample( 1, 0);

  // get slopes of lines
  double line_1_slope = line_1_y_diff/line_1_x_diff;
  double line_2_slope = line_2_y_diff/line_2_x_diff;

  // find center x
  double center_x = ( line_1_slope * line_2_slope *
                    ( sample( 0, 1) - sample( 2, 1)) +
                    line_2_slope *
                    ( sample( 0, 0) + sample( 1, 0)) -
                    line_1_slope *
                    ( sample( 1, 0) + sample( 2, 0))) /
                    (2* (line_2_slope-line_1_slope) );

  // find center y
  double center_y = -1 * (center_x - ( sample( 0, 0) + sample( 1, 0))/2) /
                    line_1_slope + (sample( 0, 1)+sample( 1, 1))/2;

  double r = sqrt( ((center_x - sample( 0, 0))*(center_x - sample( 0, 0))) + ((center_y - sample( 0, 1))*(center_y - sample( 0, 1))));

  NumericVector fit = {center_x, center_y, r};

  return fit ;

}
