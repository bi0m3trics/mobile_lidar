# CLASSIFY GROUND 

# use progressive morphological filter
# these parameters are for speed 
ws <- seq(3, 12, 4)
th <- seq(0.1, 1.5, length.out = length(ws))

# using classify_ground function from LiDR package
las <- classify_ground(las, pmf(ws, th))

# if wanting to plot
# plot(las, color = "Classification")

# use cloth simulation filter
# these parameters are also for speed
mycsf <- csf(TRUE, 1, 1, time_step = 1)
las <- classify_ground(las, mycsf)

# if wanting to plot
# plot(las, color = "Classification")
