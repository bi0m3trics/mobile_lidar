library(shiny)
library(lidR)
library(rgl)

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
    req(input$f)
    input$f
    
  })
  
  # render drop down for files that have been uploaded
  output$file_selector <- renderUI({
    files <- c(input$f$name)
    selectInput('file_selector',
                label = 'Select File (After Upload)',
                choices = files)
    
  })
  
  # use the file that is selected from the drop down
  # create plot when submit button is pressed
  plot_reactive <- eventReactive(input$submit, {
    
    # uses readLAS on user input
    las <- readLAS(input$file_selector)
    print(las)
    lidR::plot(las)
    rglwidget()
  })
  
  
  # render WebGL for 3dplot
  output$plot <- renderRglwidget({
    rgl.open(useNULL=TRUE)
    evn = parent.frame()
    plot_reactive()
  })
}

shinyApp(ui = ui, server = server)