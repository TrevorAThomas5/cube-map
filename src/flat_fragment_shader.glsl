#version 330 core
out vec4 FragColor;

in vec2 TextureCoords;

uniform sampler2D text;

void main()
{
    vec4 color = texture(text, TextureCoords); 

    if(color.a <= 0.0) {
        discard;
    }
    else {
        FragColor = texture(text, TextureCoords); 
    }
}