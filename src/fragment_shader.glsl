#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D text;

void main()
{
    FragColor = vec4(texture(text, TexCoords).rgb, 1.0f);
}
