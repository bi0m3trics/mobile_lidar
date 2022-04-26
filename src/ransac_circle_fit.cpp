#include <Rcpp.h>
#include <omp.h>
using namespace Rcpp;

#define CORES 8




// [[Rcpp::plugins(openmp)]]
//' @export
// [[Rcpp::export]]
NumericVector ransac_circle_fit(NumericMatrix points, int max, float t, float inclusion ) {

  // threshold squared to avoid extra computation later
  double t_2 = t * t;

  // core count assignment
  omp_set_num_threads(CORES);

  // number of points in the tree, needed to randomly select a point
  int num_points = (points.nrow() - 1);

  // array to hold results, each thread writes to its own row
  double results [CORES][5] = {0};

  // for loop to set each threads circle error to max int
  for( int i = 0; i < CORES; i++)
  {
    results[i][3] = INT_MAX;
  }



  // parallel for loop each thread does equal number of iterations
  int thread_i = 0;
  #pragma omp parallel for shared( points, max, t, inclusion, results) private( thread_i)
  for( thread_i = 0; thread_i < max; thread_i++ )
    {

      // thread id
      int tid=omp_get_thread_num();

      // variables for circle fit
      double candidate_fit_error = 0;
      double inclusion_percent = 0;
      double point_to_center = 0;

      // sample 3 points
      int p1 = rand() % num_points;
      int p2 = rand() % num_points;
      int p3 = rand() % num_points;

      // avoid sampling same point twice
      while( (p1 == p2) | (p1 == p3))
      {
        p1 = rand() % num_points;
      }
      while( (p2 == p3) | (p2 == p1))
      {
        p2 = rand() % num_points;
      }


      // fit circle

      // find coordinate differences on lines drawn between points
      double line_1_y_diff = points( p2, 1)  - points( p1, 1) ;
      double line_1_x_diff = points( p2, 0)  - points( p1, 0) ;
      double line_2_y_diff = points( p3, 1)  - points( p2, 1) ;
      double line_2_x_diff = points( p3, 0)  - points( p2, 0) ;

      // get slopes of lines
      double line_1_slope = line_1_y_diff/line_1_x_diff;
      double line_2_slope = line_2_y_diff/line_2_x_diff;

      // find center x
      double center_x = ( line_1_slope * line_2_slope *
                          ( points( p1, 1)  - points( p3, 1) ) +
                          line_2_slope *
                          ( points( p1, 0)  + points( p2, 0) ) -
                          line_1_slope *
                          ( points( p2, 0)  + points( p3, 0) )) /
                            (2* (line_2_slope-line_1_slope) );

      // find center y
      double center_y = -1 * (center_x - ( points( p1, 0)  + points( p2, 0) )/2) /
        line_1_slope + (points( p1, 1) +points( p2, 1) )/2;

      // find radius
      double radius = sqrt( ((center_x - points( p1, 0) )*(center_x - points( p1, 0) )) +
                            ((center_y - points( p1, 1) )*(center_y - points( p1, 1) )));


      // reset variables for a new circle fit
      inclusion_percent = 0;
      candidate_fit_error = 0;
      point_to_center = 0;

      // SIMD can be used here for distance calculation of points
      // evaluate circle fit for each point
      int error_i = 0;
      for( error_i = 0; error_i < num_points ; error_i++)
      {

        // find distance from point to center
        point_to_center = sqrt( pow( (points( error_i, 0 ) - center_x), 2)
                                  +
                                pow( (points( error_i, 1 ) - center_y), 2));


        // if the points distance from the center, minus the radius, squared
        // is less than threshold squared we increment inclusion percent
        if( ((point_to_center - radius)*(point_to_center - radius)) < t_2)
        {
          inclusion_percent += 1;
        }

        candidate_fit_error += (point_to_center - radius)*(point_to_center - radius);

      }


      // divide error sum by number of points to get mean squared error
      candidate_fit_error /=  num_points;

      // divide inclusion count by number of points to get inclusion percent
      inclusion_percent /= num_points;



      // if candidate fit error is less than current error
      // and if the inclusion percent is greater than desired inclusion
      //  we replace
      if( candidate_fit_error < results[ tid][3] && inclusion_percent > inclusion )
      {
        results[ tid][0] = center_x;
        results[ tid][1] = center_y;
        results[ tid][2] = radius;
        results[ tid][3] = candidate_fit_error;
        results[ tid][4] = inclusion_percent;
      }
    }




  // loop through to find best fit
  int best_index = 0;
  int j = 0;
  for( j = 1; j < CORES; j++)
  {
    if( results[j][3] < results[best_index][3])
    {
      best_index = j;
    }
  }

  NumericVector best_fit {results[ best_index][0],
                          results[ best_index][1],
                          results[ best_index][2],
                          results[ best_index][3],
                          results[ best_index][4]};

  return best_fit;
}
