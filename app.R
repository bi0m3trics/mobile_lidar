library(shiny)
library(lidR)

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
                 multiple = TRUE)),
  
    # main viewing pane
    mainPanel(
  
      # shows the files that are loaded in a table format
      tableOutput("table")
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
}

shinyApp(ui = ui, server = server)

