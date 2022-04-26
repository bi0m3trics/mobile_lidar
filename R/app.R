# import shiny for web front end, lidR for lidar processing functionalities
# and rgl for raster/3D graphics
library(shiny)
library(lidR)
library(rgl)
library(spanner)
library(dplyr)
library(Rcpp)
library(rospca)

#' @import shiny
#' @import lidR
#' @import rgl
#' @import spanner
#' @import dplyr
#' @import Rcpp
#' @import rospca

#' @export
app <- function(...){

  # maximum file size upload is 10 Gigs (10000*1024^2)
  options(shiny.maxRequestSize = 10000*1024^2)

  # shiny convenience wrapper for browser front end
  ui <- fluidPage(

    # Page title (essentially <h1> in browser)
    titlePanel("Mobile Lidar"),


    sidebarLayout(

      # start of stuff on sidebar
      sidebarPanel(

        # file input
        # TODO: don't allow wrong file extensions
        fileInput( inputId = "file_upload",
                   label = "Choose .las or .laz file (max file size is 10 GB)",
                   accept = c(".laz", ".laz"),
                   multiple = TRUE),

        uiOutput('file_selector'),

        actionButton("btn_clean_data", label = "Clean Data"),

        actionButton("btn_draw_point_cloud", label = "Draw Point Cloud"),

        actionButton("btn_ransac", label = "Run Ransac"),

        actionButton("btn_draw_slice", label = "Draw Slice and Table")

      ),

      # main viewing pane
      mainPanel(

        tabsetPanel(type = "tabs",

                    # shows the files that are loaded in a table format
                    tabPanel("Table View", tableOutput("file_data"),
                             verbatimTextOutput("las_data")),

                    # shows the RGL viewer for 3d point cloud
                    tabPanel("Point Cloud View", rglwidgetOutput("plot",  width = 800, height = 600)),

                    # shows the RGL viewer for slice view
                    tabPanel("Slice View", uiOutput("btn_process_data"),
                             rglwidgetOutput("slice",  width = 800, height = 600),
                             dataTableOutput("slice_table"))
        )
      )
    )
  )

  server <- function(input, output) {

    # render a table showing:
    # file name, size, a type and path
    output$file_data <- renderTable({
      req(input$file_upload)
      input$file_upload

    })

    # render drop down for files that have been uploaded
    # multiple files may be uploaded
    output$file_selector <- renderUI({
      files <- c(input$file_upload$name)
      selectInput('file_selector',
                  label = 'Select File (After Upload)',
                  choices = files)
    })

    # use the file that is selected from the drop down
    # create plot when submit button is pressed
    clean_reactive <- eventReactive(input$btn_clean_data, {


      # show pop up that data is cleaning
      showModal(modalDialog("Step 1/5: Reading file..."))

      # readTLSLAS parses into lidR-defined objects, which can be presented in plots
      # see documentation for possible parameters
      las <- readTLSLAS(input$file_selector, filter="-keep_circle 0 0 15 -thin_with_voxel 0.01")

      ## CLASSIFY GROUND ##

      showModal(modalDialog("Step 2/5: Classifying ground with cloth simulation..."))
      # use cloth simulation filter ; separates point clouds into ground and non-ground measurements
      mycsf <- csf(
        sloop_smooth = FALSE,
        class_threshold = 0.5,
        cloth_resolution = 0.5,
        rigidness = 1L,
        iterations = 500L,
        time_step = 0.65)

      # classify ground
      las <- classify_ground(las, mycsf)


      showModal(modalDialog("Step 3/5: Normalizing height..."))
      # normalize height with tin
      las <- normalize_height(las, tin())


      showModal(modalDialog("Step 4/5: Classifying noise..."))
      # classify noise using IVF algorithm ivf with 1 meter voxel, 3 points near
      las <- classify_noise(las, ivf(1, 3))


      showModal(modalDialog("Step 5/5: Filtering points..."))

      # Remove outliers using filter_poi()
      las <<- filter_poi(las, Classification != LASNOISE)

      # print summary of las file to table
      summary(las)

      # remove pop up window
      removeModal()

    })


    # output to las_data the call from clean_reative()and render table of summary(las)
    output$las_data <- renderPrint({
      clean_reactive()
    })



    # use the file that is selected from the drop down
    # create plot when submit button is pressed
    plot_reactive <- eventReactive(input$btn_draw_point_cloud, {

      # plot the non-ground points, colored by height
      lidR::plot(voxelize_points(filter_poi(las, Classification!=2), 0.25), color="Z", breaks = "quantile")
      rglwidget()
    })


    # render WebGL widget window, then call above plot_reactive to render
    # the lidar visualization
    output$plot <- renderRglwidget({
      plot_reactive()
    })


    ransac_reactive <- eventReactive(input$btn_ransac, {
      # show pop up that data segmenting
      showModal(modalDialog("Step 1/3: Setting up segmentation..."))

      # set up for dbscan to find treeIDs
      las_slice <- filter_poi(las, Z>=0.5, Z<=2)

      # use spanner for setup
      eigens <- spanner::eigen_metrics(las_slice, radius = 0.33, ncpu = 8)
      pt_den <- spanner:::C_count_in_sphere(las_slice, radius = 0.33, ncpu = 8)
      las_slice@data<-cbind(las_slice@data, eigens)
      las_slice@data<-cbind(las_slice@data, pt_den)

      las_slice <- filter_poi(las_slice, Z>=0.87, Z<=1.87)

      showModal(modalDialog("Step 2/3: Clustering points with dbscan..."))

      # cluster points using dbscan
      clust <- dbscan::dbscan(las_slice@data[,c("X","Y","Z", "eSum","Verticality")], eps = 0.25, minPts = 100)

      # create new column treeID
      las_slice@data$treeID<-clust$cluster

      # las_slice_data is a dataframe with only important points for ransac later on
      las_slice_data <- las_slice@data[,c("X", "Y", "Z", "treeID")]

      showModal(modalDialog("Step 3/3: Ransac circle fitting..."))

      # call ransac fit function
      fit_df <<- las_slice_circle_fitting( las_slice, 1000, 0.05, 0.85)

      fit_df

      # remove pop up window
      removeModal()
    })


    # render table of data points
    output$slice_table <- renderDataTable({
      ransac_reactive()
    })


    # use the file that is selected from the drop down
    # create plot when submit button is pressed
    slice_reactive <- eventReactive(input$btn_draw_slice, {

      output$slice_table <- renderDataTable(fit_df)

      # plot the slice
      offsets <- lidR::plot(las_slice, color="treeID", axis = T)
      spheres3d( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = 1.37, r = fit_df[,3], alpha = .7)

      rgl.texts( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = fit_df[,3]+1.37 +.1, text = paste( "x: ",fit_df[,1]))
      rgl.texts( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = fit_df[,3]+1.37 +.2, text = paste( "y: ", fit_df[,2] ))
      rgl.texts( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = fit_df[,3]+1.37 +.3, text = paste( "diameter: ", fit_df[,3]*2))
      rgl.texts( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = fit_df[,3]+1.37 +.4, text = paste( "mean squared error: ", fit_df[,4] ))
      rgl.texts( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = fit_df[,3]+1.37 +.5, text = paste( "inclusion: ", fit_df[,5] ))
      rgl.texts( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = fit_df[,3]+1.37 +.6, text = paste( "tree id: ", fit_df[,6] ))
      rgl.texts( x = fit_df[,1]-offsets[1], y = fit_df[,2]-offsets[2], z = fit_df[,3]+1.37 +.7, text = paste( "lean: ", fit_df[,7] ))
      rglwidget()


    })


    # render WebGL widget window, then call above plot_reactive to render
    # the lidar visualization
    output$slice <- renderRglwidget({
      slice_reactive()
    })
  }

  shinyApp(ui = ui, server = server)
}
