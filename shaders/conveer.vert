#version 330 core
layout (location = 0) in vec2 aPos; // Normalized quad vertex (-1 to 1)

uniform vec2 points[4]; // 0: bottom-left, 1: bottom-right, 2: top-right, 3: top-left

out vec2 TexCoords;

void main()
{
    // Simple interpolation based on the input quad vertices.
    // This maps the corners of the input quad (-1,-1) etc. to the custom points.
    vec2 finalPos = mix(
        mix(points[3], points[0], (aPos.y + 1.0) / 2.0), // Interpolate left side
        mix(points[2], points[1], (aPos.y + 1.0) / 2.0), // Interpolate right side
        (aPos.x + 1.0) / 2.0                                 // Interpolate between left and right
    );

    gl_Position = vec4(finalPos, 0.0, 1.0);
    TexCoords = (aPos + 1.0) / 2.0; // Pass through texture coordinates
}