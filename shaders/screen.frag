#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 windowSize;

void main()
{
    FragColor = texture(screenTexture, TexCoords);
}