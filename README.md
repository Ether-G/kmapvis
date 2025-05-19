# K-Map Visualizer

An interactive Karnaugh Map visualizer built with OpenGL that helps visualize and understand K-map optimization techniques.

## Features

- Interactive 2D visualization of K-maps (2-4 variables)
- Real-time drawing of K-map loops (implicants)
- Support for both SOP (Sum of Products) and POS (Product of Sums) forms
- Step-by-step visualization of the optimization process
- Modern OpenGL rendering with shader support

## Dependencies

- C++17 or later
- CMake 3.15 or later
- OpenGL
- GLFW3
- GLAD
- GLM
- ImGui

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make

# Run the application
./kmapvis
```

## Usage

- Left click and drag to draw K-map loops
- Right click to remove loops
- Mouse wheel to zoom in/out
- Middle mouse button to pan
- Space to clear all loops
- Number keys 2-4 to change number of variables

## License

MIT License 