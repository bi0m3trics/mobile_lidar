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
  titlePanel("Lumberhack"),
  
  
  sidebarLayout(
    
    # start of stuff on sidebar
    sidebarPanel(
      
      # file input
      fileInput( inputId = "fileIn",
                 label = "Choose .las or .laz file (max file size is 10 GB)",
                 accept = c(".laz", ".laz"),
                 multiple = TRUE),
      
      uiOutput('file_selector'),
      
      actionButton("submit", label = "Submit")
    ),
    
    # main viewing pane
    mainPanel(
      
      # shows the files that are loaded in a table format
      tableOutput("table"),
      
      # shows the RGL viewer for 3d point cloud
      rglwidgetOutput("plot",  width = 800, height = 600)
    )
  )
)

server <- function(input, output) {
  
  # render a table showing:
  # file name, size, a type and path 
  output$table <- renderTable({ 
    req(input$fileIn)
    input$fileIn
    
  })
  
  # render drop down for files that have been uploaded
  # multiple files may be uploaded
  output$file_selector <- renderUI({
    files <- c(input$fileIn$name)
    selectInput('file_selector',
                label = 'Select File (After Upload)',
                choices = files)
    
  })
  
  # use the file that is selected from the drop down
  # create plot when submit button is pressed
  plot_reactive <- eventReactive(input$submit, {
    
    # uses readLAS on user input
      # readLAS parses into lidR-defined objects, which can be presented in plots
      # see documentation for possible parameters
    las <- readLAS(input$file_selector)
    print(las)
    
    # render the 3-D lidar visualization
    lidR::plot(las)
    rglwidget()
  })
  
  
  # render WebGL widget window, then call above plot_reactive to render
  # the lidar visualization
  output$plot <- renderRglwidget({
    rgl.open(useNULL=TRUE)
    evn = parent.frame()
    plot_reactive()
  })
}

shinyApp(ui = ui, server = server)
