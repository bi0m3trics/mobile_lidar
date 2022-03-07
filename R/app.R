# import shiny for web front end, lidR for lidar processing functionalities
# and rgl for raster/3D graphics
library(shiny)
library(lidR)
library(rgl)
library(spanner)
library(dplyr)
library(Rcpp)


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

        radioButtons("point_cloud_view", "Point Cloud View", c("Simple" = "simple", "Segment Trees (Takes a while to process)" = "segment")),

        actionButton("btn_draw_point_cloud", label = "Draw Point Cloud"),

        actionButton("btn_draw_slice", label = "Draw Slice")

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
                    tabPanel("Slice View", rglwidgetOutput("slice",  width = 800, height = 600), dataTableOutput("slice_table"))
        )
      )
    )
  )

  server <- function(input, output) {

    las <- NULL

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
      showModal(modalDialog("Cleaning data..."))

      # readTLSLAS parses into lidR-defined objects, which can be presented in plots
      # see documentation for possible parameters
      las <<- readTLSLAS(input$file_selector, filter="-keep_circle 0 0 15 -thin_with_voxel 0.01")

      ## CLASSIFY GROUND ##

      # use cloth simulation filter ; separates point clouds into ground and non-ground measurements
      mycsf <- csf(
        sloop_smooth = FALSE,
        class_threshold = 0.5,
        cloth_resolution = 0.5,
        rigidness = 1L,
        iterations = 500L,
        time_step = 0.65)

      # classify ground
      las <<- classify_ground(las, mycsf)

      # normalize height with tin
      las <<- normalize_height(las, tin())


      # classify noise using IVF algorithm ivf with 1 meter voxel, 3 points near
      las <<- classify_noise(las, ivf(1, 3))

      # Remove outliers using filter_poi()
      las <<- filter_poi(las, Classification != LASNOISE)

      # print summary of las file to table
      output$las_data <- renderPrint(lidR::summary(las))

      # remove pop up window
      removeModal()

    })


    # output to las_data the call from clean_reative()and render table of summary(las)
    output$las_data <- renderText({
      clean_reactive()
    })



    # use the file that is selected from the drop down
    # create plot when submit button is pressed
    plot_reactive <- eventReactive(input$btn_draw_point_cloud, {


      if(input$point_cloud_view == "simple")
      {
        # plot the non-ground points, colored by height
        lidR::plot(las, color="Z")
      }

      else
      {
        # perform a deep inspection of the las object. If you see any
        # red text, you may have issues!
        las_check(las)

        # find individual tree locations and attribute data
        myTreeLocs = get_raster_eigen_treelocs(las = las, res = 0.05,
                                               pt_spacing = 0.0254,
                                               dens_threshold = 0.2,
                                               neigh_sizes=c(0.333, 0.166, 0.5),
                                               eigen_threshold = 0.5,
                                               grid_slice_min = 0.6666,
                                               grid_slice_max = 2.0,
                                               minimum_polygon_area = 0.025,
                                               cylinder_fit_type = "ransac",
                                               output_location = getwd(),
                                               max_dia=0.5,
                                               SDvert = 0.25)

        # plot the tree information over a CHM
        plot(lidR::grid_canopy(las, res = 0.2, p2r()))
        points(myTreeLocs$X, myTreeLocs$Y, col = "black", pch=16,
               cex = myTreeLocs$Radius^2*10, asp=1)


        # segment the point cloud
        myTreeGraph <- segment_graph(las = las, tree.locations = myTreeLocs, k = 50,
                                     distance.threshold = 0.5,
                                     use.metabolic.scale = FALSE,
                                     subsample.graph = 0.1,
                                     return.dense = FALSE,
                                     output_location = getwd())

        # plot it in 3d colored by treeID
        plot(myTreeGraph, color = "treeID")
      }
      rglwidget()
    })


    # render WebGL widget window, then call above plot_reactive to render
    # the lidar visualization
    output$plot <- renderRglwidget({
      plot_reactive()
    })


    # use the file that is selected from the drop down
    # create plot when submit button is pressed
    slice_reactive <- eventReactive(input$btn_draw_slice, {

      # show pop up that data is cleaning
      showModal(modalDialog("Slicing and segmenting data..."))

      # set up for dbscan to find treeIDs
      las_slice <- filter_poi(las, Z>=0.5, Z<=2)

      # use spanner for setup
      eigens <- spanner::eigen_metrics(las_slice, radius = 0.33, ncpu = 8)
      pt_den <- spanner:::C_count_in_sphere(las_slice, radius = 0.33, ncpu = 8)
      las_slice@data<-cbind(las_slice@data, eigens)
      las_slice@data<-cbind(las_slice@data, pt_den)

      # get slice from 0.87m to 1.87m
      las_slice <- filter_poi(las_slice, Z>=0.87, Z<=1.87)

      # cluster points and assign treeID using dbscan
      clust <- dbscan::dbscan(las_slice@data[,c("X","Y","Z", "eSum","Verticality")], eps = 0.25, minPts = 100)

      # create new column treeID
      las_slice@data$treeID<-clust$cluster

      # las_slice_data is a dataframe that only includes necessary information to pass to C++
      las_slice_data <- las_slice@data[,c("X", "Y", "Z", "treeID")]

      # render table of data points
      output$slice_table <- renderDataTable(las_slice_data)

      # write to csv for testing
      # write_csv(las_slice@data[,c("X", "Y", "Z", "treeID")],"C:\\Users\\rexmy\\Documents\\GitHub\\mobile_lidar\\DensePatchA.csv")

      # remove pop up window
      removeModal()

      # plot the slice
      lidR::plot(las_slice, color="treeID")


      # test cpp function "ransac" (just returns df for now)
      print(rcpp_ransac(las_slice_data))

      rglwidget()

    })


    # render WebGL widget window, then call above plot_reactive to render
    # the lidar visualization
    output$slice <- renderRglwidget({
      slice_reactive()
    })
  }

  shinyApp(ui = ui, server = server, ...)
}
