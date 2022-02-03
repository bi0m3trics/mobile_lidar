
# ensures that the path and files given are valid
# checks for directory and path existence, and correct file extensions
validate_files <- function (path, filenames) {

	# strip leading and trailing whitespace
	path = trimws(path)

	# make sure directory exists on filesystem
	if ( !file.exists(path) ) {
		return ("DIR_NONEXISTENT")
	}
	
	# make sure each specified file exists in the specified directory
	for (filename in filenames) {
		if ( !file.exists( file.path(path, filename) ) ) {
			return ("FILE_NONEXISTENT")
		}	

		file_extension <- substr( filename, nchar(filename) - 2, nchar(filename) )
		if (file_extension != "las" & file_extension != "laz") {
			return ("WRONG_EXTENSION")
		}
	}

	return (TRUE)
}

