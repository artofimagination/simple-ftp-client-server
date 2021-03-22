project(FileProcessor)

message("Compile FileProcessor")

# Main source for the library  
set(MAIN_SOURCES
		${PROJECT_SOURCE_DIR}/BasicFileProcessor.cpp		
		${PROJECT_SOURCE_DIR}/BasicFileProcessor.h
)

# create library and add dependencies 
add_library(FileProcessor SHARED ${MAIN_SOURCES} )
install(TARGETS FileProcessor DESTINATION $ENV{OUTPUT_PATH})
set_property(TARGET FileProcessor PROPERTY CXX_STANDARD 17)

target_link_libraries(FileProcessor LINK_PUBLIC ${LIBRARIES})