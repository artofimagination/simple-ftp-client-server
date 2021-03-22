project(Common)

message("Compile Common")

# Main source for the library  
set(MAIN_SOURCES
		${PROJECT_SOURCE_DIR}/GeneralFactory.cpp		
		${PROJECT_SOURCE_DIR}/GeneralFactory.h
		${PROJECT_SOURCE_DIR}/CommandDefs.h
		${PROJECT_SOURCE_DIR}/Error.h
)

set(LIBRARIES
				UI)

# create library and add dependencies 
add_library(Common SHARED ${MAIN_SOURCES} )
install(TARGETS Common DESTINATION $ENV{OUTPUT_PATH})
set_property(TARGET Common PROPERTY CXX_STANDARD 17)

target_link_libraries(Common LINK_PUBLIC ${LIBRARIES})