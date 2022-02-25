library(shiny)
library(lidR)
library(rgl)
library(spanner)
library(dplyr)

source("file_upload.R")

ui <- fluidPage(
  
  # Page title (essentially <h1> in browser)
  titlePanel("Mobile Lidar"),
  
  
  sidebarLayout(
    
    # start of stuff on sidebar
    sidebarPanel(
      
      fileInput( inputId = "file_upload",
                 label = "Choose .las or .laz file (max file size is 10 GB)",
                 accept = c(".laz", ".las"),
                 multiple = TRUE
			   ),
      
	  textInput("file_path", "Please specify the directory path in which the files are lcoated"),
	  actionButton("submit_filepath", "Submit"),
	  textOutput("upload_status"),

      uiOutput("file_selector"),
      
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
        tabPanel("Slice View", rglwidgetOutput("slice",  width = 800, height = 600),
                              dataTableOutput("slice_data"))
      )
    )
  )
)