project(ServerApp)

# Main source for the library 
set(MAIN_SOURCES 
		${PROJECT_SOURCE_DIR}/Server.cpp
		${PROJECT_SOURCE_DIR}/Server.h
		${PROJECT_SOURCE_DIR}/ServerBuilder.cpp
		${PROJECT_SOURCE_DIR}/ServerBuilder.h
		${PROJECT_SOURCE_DIR}/ThreadController.cpp		
		${PROJECT_SOURCE_DIR}/ThreadController.h
		${PROJECT_SOURCE_DIR}/ClientHandler.cpp		
		${PROJECT_SOURCE_DIR}/ClientHandler.h
		${PROJECT_SOURCE_DIR}/main.cpp
		)

# Setup parameters common between unit test and library
# Set custom libraries
set(LIBRARIES
				Common
				Network
				FileProcessor)

add_executable(ServerApp ${MAIN_SOURCES})
install(TARGETS ServerApp DESTINATION .)

# Linking libraries.
target_link_libraries(ServerApp LINK_PUBLIC ${LIBRARIES})

