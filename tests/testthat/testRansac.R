
# Test that the number of trees that is sent to ransac function is the same
# number that is output from it
test_that("number of tree returns is number of tree inputs", {
  expect_equal(max(las_slice@data$treeID), max(fit_df[,6]))
})
