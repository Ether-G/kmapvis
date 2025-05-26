cmake_minimum_required(VERSION 3.10)
project(kmap_solver)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 package with 3D modules
find_package(Qt5 COMPONENTS Widgets 3DCore 3DRender 3DExtras 3DInput REQUIRED)

# Add executable for terminal version
add_executable(kmap_solver
    main.cpp
    kmap_solver.cpp
)

# Add executable for GUI version
add_executable(kmap_solver_gui
    main_gui.cpp
    kmap_solver.cpp
    kmap_gui.cpp
    kmap_gui.hpp
)

# Include directories
target_include_directories(kmap_solver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(kmap_solver_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Qt libraries
target_link_libraries(kmap_solver_gui PRIVATE 
    Qt5::Widgets 
    Qt5::3DCore 
    Qt5::3DRender 
    Qt5::3DExtras 
    Qt5::3DInput
) 