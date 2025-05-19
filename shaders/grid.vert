#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform vec2 cellPosition;
uniform vec2 cellSize;

void main() {
    // Calculate cell position and scale
    vec2 pos = aPos * cellSize + cellPosition;
    
    // Center the K-map and scale it to a reasonable size
    pos = (pos - 0.5) * 0.8;  // Scale to 80% of the screen and center
    
    gl_Position = projection * view * vec4(pos, 0.0, 1.0);
    TexCoord = aTexCoord;
} 