## CLASSIFY GROUND ## 

# use progressive morphological filter ; detects non-ground LIDAR measurements
# these parameters are for speed 
ws <- seq(3, 12, 4)
th <- seq(0.1, 1.5, length.out = length(ws))

# using classify_ground function from LiDR package
las <- classify_ground(las, pmf(ws, th))

# if wanting to plot
# plot(las, color = "Classification")

# use cloth simulation filter ; separates point clouds into ground and non-ground measurments 
# these parameters are also for speed
mycsf <- csf(TRUE, 1, 1, time_step = 1)
las <- classify_ground(las, mycsf)

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
