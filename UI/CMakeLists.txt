project(UI)

message("Compile UI")

# Main source for the library  
set(MAIN_SOURCES
		${PROJECT_SOURCE_DIR}/CommandLineImpl.cpp		
		${PROJECT_SOURCE_DIR}/CommandLineImpl.h
)

# create library and add dependencies 
add_library(UI SHARED ${MAIN_SOURCES} )
install(TARGETS UI DESTINATION $ENV{OUTPUT_PATH})
set_property(TARGET UI PROPERTY CXX_STANDARD 17)

target_link_libraries(UI LINK_PUBLIC ${LIBRARIES})