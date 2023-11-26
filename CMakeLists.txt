# CMakeLists.txt for your project

cmake_minimum_required(VERSION 3.10)
project(lanplay-server-mgr)

# Add your project's source files
set(SOURCE_FILES
    main.cpp
    # Add more source files as needed
)

# Include lan-play header files
# include_directories(path_to_lan_play/include)

# Link your project with lan-play library
# link_directories(path_to_lan_play/build/lib)
add_executable(lanplay-server-mgr ${SOURCE_FILES})
#target_link_libraries(your_project lanplay)
INSTALL(TARGETS lanplay-server-mgr RUNTIME DESTINATION bin)
