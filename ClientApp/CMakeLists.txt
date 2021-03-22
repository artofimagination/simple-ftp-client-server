project(ClientApp)

# Main source for the library 
set(MAIN_SOURCES 
		${PROJECT_SOURCE_DIR}/Client.cpp
		${PROJECT_SOURCE_DIR}/Client.h
		${PROJECT_SOURCE_DIR}/ClientBuilder.cpp
		${PROJECT_SOURCE_DIR}/ClientBuilder.h
		${PROJECT_SOURCE_DIR}/main.cpp
		)

# Setup parameters common between unit test and library
# Set custom libraries
set(LIBRARIES
				UI
				Common
				Network
				FileProcessor)

add_executable(ClientApp ${MAIN_SOURCES})
install(TARGETS ClientApp DESTINATION .)

# Linking libraries.
target_link_libraries(ClientApp LINK_PUBLIC ${LIBRARIES})

