library(shiny)
library(lidR)
library(shinyRGL)

# maximum file size upload is 10 Gigs (10000*1024^2)
options(shiny.maxRequestSize = 10000*1024^2)

ui <- fluidPage(
  
  # we need a name
  titlePanel("Lumberhack"),
  
  
  sidebarLayout(
    
    # start of stuff on sidebar
    sidebarPanel(
      
      # file input
      fileInput( inputId = "f",
                 label = "Choose .las or .laz file (max file size is 10 GB)",
                 accept = c(".laz", ".laz"),
                 multiple = TRUE),
      
      actionButton("submit", label = "Submit")
    ),
    
    # main viewing pane
    mainPanel(
      
      # shows the files that are loaded in a table format
      tableOutput("table"),
      
      # shows the RGL viewer for 3d point cloud
      webGLOutput("RGL")
    )
  )
)

server <- function(input, output) {
  
  # render a table showing:
  # file name, size, a type and path 
  output$table <- renderTable({ 
    req(input$f)
    input$f
  })
  
  
  # DOES NOT WORK WITH USER FILE AS OF NOW
  # create plot when submit button is pressed
  plot_reactive <- eventReactive(input$submit, {
    
    # THIS NEEDS TO BE FIXED TO WORK WITH USER DATA
    # input$f uploads as a data frame, not .laz/las file
    las <- readLAS("results1.laz")
    print(las)
    lidR::plot(las)
  })
  
  
  # render WebGL for 3dplot
  output$RGL <- renderWebGL({ 
    plot_reactive()
  })
}

shinyApp(ui = ui, server = server)