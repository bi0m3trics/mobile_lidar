# import shiny for web front end, lidR for lidar processing functionalities
# and rgl for raster/3D graphics
library(shiny)
library(lidR)
library(rgl)

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
      
      actionButton("btn_draw_point_cloud", label = "Draw Point Cloud")
    ),
    
    # main viewing pane
    mainPanel(
      
      tabsetPanel(type = "tabs",
      
        # shows the files that are loaded in a table format
        tabPanel("Table View", tableOutput("file_data"),
                              verbatimTextOutput("las_data")),
        
        # shows the RGL viewer for 3d point cloud
        tabPanel("Point Cloud View", rglwidgetOutput("plot",  width = 800, height = 600))
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
  
  
  # THIS NEEDS WORK
  # Warning: Invalid data: 10105212 points with a return number equal to 0 found.
  
  # use the file that is selected from the drop down
  # create plot when submit button is pressed
  clean_reactive <- eventReactive(input$btn_clean_data, {
    
    showModal(modalDialog("Cleaning data..."))
    
    # uses readLAS on user input
    # readLAS parses into lidR-defined objects, which can be presented in plots
    # see documentation for possible parameters
    las <- readLAS(input$file_selector)
    summary(las)
    
    # if dataset is 'clean', artificial outliers will be added
    set.seed(314)
    id = round(runif(20, 0, npoints(las)))
    set.seed(42)
    err = runif(20, -50, 50)
    las$Z[id] = las$Z[id] + err
    
    # classify noise using SOR algorithm
    las <- classify_noise(las, sor(15,7))
    
    # plot
    (las, color = "Classification")
    
    # use IVF algorithm 
    las <- classify_noise(las, ivf(5, 2))

    # Remove outliers using filter_poi()
    las_denoise <- filter_poi(las, Classification != LASNOISE)
    
    summary(las_denoise)
    removeModal()
   
  })
  
  
  # render WebGL widget window, then call above plot_reactive to render
  # the lidar visualization
  output$las_data <- renderPrint({
    clean_reactive()
  })
  
  
  
  # use the file that is selected from the drop down
  # create plot when submit button is pressed
  plot_reactive <- eventReactive(input$btn_draw_point_cloud, {
    rgl.open(useNULL=T)
    
    las = readLAS(input$file_selector)
    
    las2 = voxelize_points(las, 0.5)
    plot(las2)
    rglwidget()
    
  })
  
  
  # render WebGL widget window, then call above plot_reactive to render
  # the lidar visualization
  output$plot <- renderRglwidget({
    plot_reactive()
  })
}

shinyApp(ui = ui, server = server)
