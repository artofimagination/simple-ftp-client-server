project(Network)

message("Compile Network")

find_package (Threads REQUIRED)

# Main source for the library  
set(MAIN_SOURCES
		${PROJECT_SOURCE_DIR}/BasicNetwork.cpp		
		${PROJECT_SOURCE_DIR}/BasicNetwork.h
)

set(LIBRARIES
				pthread)

# create library and add dependencies 
add_library(Network SHARED ${MAIN_SOURCES} )
install(TARGETS Network DESTINATION $ENV{OUTPUT_PATH})
set_property(TARGET Network PROPERTY CXX_STANDARD 17)

target_link_libraries(Network LINK_PUBLIC ${LIBRARIES})