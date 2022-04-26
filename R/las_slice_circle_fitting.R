#' slice circle fit (PCA / bisecting cords)
#'
#' this function takes in a las slice from dbscan that has labled points to tree ids and returns a dataframe
#' of tree ids, the x, y, and z corrdiantes for the circle fit on a slice, the radius fo the circle fit, the
#' mean squared error of the points and the percent inclusion of each point within the tree
#'
#' It preforms ransac circle fitting by:
#' 1. transform the las objest into a matrix
#' 2. loop through the tree ids one at a time
#' 3. prefrom robust pca using package rospca to find tree slice centers and eigenvectors(loadings)
#' 4. use center and loadings to project the 3d point cloud onto a 2d principal component space
#' 5. call the rcpp function ransac circle fit to preform circle fitting in parallel with given parameters
#' 6. return a data frame that contains tree information
#'
#' @param las_slice A slice from dbscan that has points labled with tree ids
#' @param iterations the max number of iterations to preform ransac circle fits
#' @param threshold the minimum distance a point needs to be within the circle circumference to be considered an inlier
#' @param inclusion the desired inclusion to be considered a good circle fit to the data
#' @export
las_slice_circle_fitting <- function( las_slice, iterations, threshold, inclusion)

{

  # dataframe to hold results
  fit_results <- matrix(ncol = 7, nrow = 0)
  # fit_results <- data.frame(
  #   tree_id = c(0.0),
  #   center_x = c(0.0),
  #   center_y = c(0.0),
  #   radius = c(0.0),
  #   candidate_fit_error = c(0.0),
  #   inclusion_percent = c(0.0),
  # )

  # matrix to hold points
  M <- cbind( las_slice$X, las_slice$Y, las_slice$Z, las_slice$treeID)

  # loop through all individual tree ids
  for( tree_id in 1:length(unique(las_slice$treeID)))
  {
    # select current tree
    current_pts <- M[ M[ ,4] ==  tree_id, ]

    if( nrow(current_pts) < 3)
    {
      next
    }

    # trim away tree id
    current_pts <- current_pts[ , 1:3]

    # preform robust pca
    tree_pca <- robpca(current_pts, k=3, ndir = 5000)


    # use robust pca to reduce dimensions and fit points to a plane orthogonal to the tree lean
    # get mean and principal components of points
    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    #
    # !!! note here pc1 will be the greatest value of the principal componets
    # ideally the z componet
    # basically this translates as if the diameter of the tree is close to the height of
    # the slice taken it will cause the PCA to make no sense, as it will reduce dimensions
    # in the x or y plane and not z
    #
    # the workaround is to simply take a taller tree slice
    #
    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    center <- tree_pca$center
    pc1 <- tree_pca$loadings[ , 1]
    pc2 <- tree_pca$loadings[ , 2]
    pc3 <- tree_pca$loadings[ , 3]

    # note these next lines can be used to change the slice height
    # to get more accurate results
    # filter tmp to around dbh
    # current_pts <- current_pts[ current_pts[ ,3] >=  1.07, ]
    # current_pts <- current_pts[ current_pts[ ,3] <=  1.67, ]

    # bind the lower two principal components into a matrix
    pc <- t(cbind( pc3, pc2))

    # subtract means from each point
    tmp_sub_means <- t( sweep( current_pts, 2, center ))

    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    # note: this is a matrix that has NA values as the row
    # which can cause issues with the circle fitting later
    # it has currently been worked around by skipping the first row
    # but by subtracting this row and fixing the iterations
    # to start at zero in the ransca_circle_fit() it loops infinitly
    # still working on fix
    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    # init empty matrix
    pts_proj <- matrix( nrow = 2)


    # loop through all points passing them through matrix
    # and transform them from 3d space to 2d space
    for( i in 1:ncol(tmp_sub_means))
    {
      a <- pc %*% tmp_sub_means[ , i ]
      pts_proj <- cbind(pts_proj, a)
    }

    # transpose matrix to long format
    pts_proj <- t(pts_proj)
    # remove the first entry of 0 values
    pts_proj <- pts_proj[ -c(1),]

    # call ransac to get a best fit
    best_fit <- ransac_circle_fit( pts_proj, iterations, threshold, inclusion)
    # return to xyz coordinates
    xyz_center <- center +  best_fit[1]%*%pc3 + best_fit[2]%*%pc2
    # replace pc values with xy values
    best_fit[1] <- xyz_center[1]
    best_fit[2] <- xyz_center[2]
    best_fit[6] <- tree_id
    best_fit[7] <- acos( pc1[3] / sqrt( pc1[1]^2 + pc1[2]^2 + pc1[3]^2 ))

    fit_results <- rbind( fit_results, best_fit)

    # call ransac circle fit and bind it to the data frame
    # fit_results <- rbind( fit_results, ransac_circle_fit( pts_proj, iterations, threshold, inclusion))



  }
  return(fit_results)
}
