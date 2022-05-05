#' Function updates a tree ransac fit in a given data frame
#'
#' Takes in a data frame of tree fits from las_slice_circle_fitting, a tree id and ransac parameters.
#' Does a ransac fit for the selected tree and updates the returned dataframe
#'
#' See las_slice_circle_fitting for information on how the fit is performed
#'
#' @param data a data frame object that contains tree ransac fits from las_slice_circle_fitting function
#' @param tree_id the tree id to preform ransac circle fitting
#' @param las_slice A slice from dbscan that has points labled with tree ids
#' @param iterations the max number of iterations to preform ransac circle fits
#' @param threshold the minimum distance a point needs to be within the circle circumference to be considered an inlier
#' @param inclusion the desired inclusion to be considered a good circle fit to the data
#' @export
individual_tree_ransac <- function( data, tree_id, las_slice, iterations, threshold, inclusion)
{
  # select the points from the las slice that have the tree id
  tree_slice <- filter_poi(las_slice, treeID == tree_id)

  # call las_slice_circle_fitting on tree slice to get a new fit with given parameters
  new_fit <- las_slice_circle_fitting( tree_slice, iterations, threshold, inclusion )

  data <<- data[!(data$Tree_ID == tree_id )]

  data[data$Tree_ID == tree_id,] <- new_fit

  return( data)

}
