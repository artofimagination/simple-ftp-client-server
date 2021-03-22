cmake_minimum_required(VERSION 3.10.2)
project(FileService)
  
set(TOP_LEVEL ${PROJECT_SOURCE_DIR})
# Redirect executable output
set(EXECUTABLE_OUTPUT_PATH $ENV{OUTPUT_PATH})
# Set static linking
set(STATIC_LINKING FALSE)	

# Qt Setup ----------------------------------------
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_FIND_LIBRARY_PREFIXES lib)
set(CMAKE_CXX_STANDARD 17)

include_directories ("${PROJECT_SOURCE_DIR}") 

if("${APP}" STREQUAL "Client")
	message("Compiling ClientApp")
	add_subdirectory (UI)
	add_subdirectory (Common)
	add_subdirectory (Network)
	add_subdirectory (FileProcessor)
	add_subdirectory (ClientApp)
	set_target_properties(ClientApp PROPERTIES OUTPUT_NAME "FileServiceClient")
elseif("${APP}" STREQUAL "Server")
	message("Compiling ServerApp")
	add_subdirectory (UI)
	add_subdirectory (Common)
	add_subdirectory (Network)
	add_subdirectory (FileProcessor)
	add_subdirectory (ServerApp)
	set_target_properties(ServerApp PROPERTIES OUTPUT_NAME "FileServiceServer")
endif()

