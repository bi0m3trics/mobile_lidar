## CLASSIFY GROUND ## 

# use cloth simulation filter ; separates point clouds into ground and non-ground measurements 
mycsf <- csf(
          sloop_smooth = FALSE,
          class_threshold = 0.1,
          cloth_resolution = 0.1,
          rigidness = 1L, 
          iterations = 500L,
          time_step = 0.65)

# classify ground
las <- classify_ground(las, mycsf)

# normalize height with tin
las <- normalize_height(las, tin())

# decimate points
# filters point cloud by randomly selecting n points with each voxel
thinned <- decimate_points(las, random_per_voxel(1, 1))

# if wanting to plot
# plot(las, color = "Classification")

## CLASSIFY CROWN ##
# NEEDS WORK 

# using k-means cluster for individual tree detection
# set xyz coordinates and subset the data
xyzi <- subset(las[,1:4], las[,3] >= 1.37)

# find if there are any tree clusters in data
tclas <- kmeans(xyzi[,1:2], 32)

# set id vector on tree
ID <- as.factor(tclas$cluster)

# combining xyzi and tree id
xyziID <- cbind(xyzi, ID)

# compute individual tree crown metrics
treeCrown <- CrownMetrics(xyziID)
head(treeCrown)
