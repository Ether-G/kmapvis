#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform vec4 gridColor;
uniform vec4 cellColor;
uniform vec4 loopColor;
uniform vec4 essentialColor;
uniform int cellState;

void main() {
    // Draw grid lines
    float gridLine = 0.0;
    if (mod(TexCoord.x, 0.25) < 0.01 || mod(TexCoord.y, 0.25) < 0.01) {
        gridLine = 1.0;
    }

    // Determine cell color based on state
    vec4 finalColor;
    switch (cellState) {
        case 0: // UNMARKED
            finalColor = cellColor;
            break;
        case 1: // MARKED
            finalColor = mix(cellColor, vec4(0.0, 0.0, 1.0, 1.0), 0.5);
            break;
        case 2: // IN_LOOP
            finalColor = mix(cellColor, loopColor, 0.5);
            break;
        case 3: // ESSENTIAL
            finalColor = mix(cellColor, essentialColor, 0.5);
            break;
        default:
            finalColor = cellColor;
    }

    // Mix grid lines with cell color
    FragColor = mix(finalColor, gridColor, gridLine);
} 