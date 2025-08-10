#version 330 core
out vec4 FragColor;

in vec2 TexCoords; // Input from the vertex shader

void main()
{
    // Using TexCoords to create a simple pattern instead of a solid color
    FragColor = vec4(TexCoords.x, TexCoords.y, 0.5, 1.0);
}