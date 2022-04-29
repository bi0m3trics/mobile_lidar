library(stringr)
library(tools)

# Test that file types are laz for our test files
test_that("file type is of .las or .laz", {
  expect_equal(file_ext("DensePatchA.laz"), "laz")
  expect_equal(file_ext("results1.laz"), "laz")
  expect_equal(file_ext("results2.laz"), "laz")
})
