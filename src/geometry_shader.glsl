#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;


in VS_OUT {
    vec2 TexCoords;
} gs_in[];


//out vec3 dFragPos;
//out vec3 dNormal;
out vec2 dTexCoords;

//out vec3 color;

void main() {

    gl_Position = gl_in[0].gl_Position;
    dTexCoords = gs_in[0].TexCoords;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    dTexCoords = gs_in[1].TexCoords;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    dTexCoords = gs_in[2].TexCoords;
    EmitVertex();

    /*
    gl_PointSize = 10.0f;
    gl_Position = gl_in[0].gl_Position;
    color = gs_in[0].color;
    EmitVertex();
    */

    /*
    gl_Position = gl_in[0].gl_Position;
    dFragPos = gs_in[0].FragPos;
    dNormal = gs_in[0].Normal;
    dTexCoords = gs_in[0].TexCoords;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    dFragPos = gs_in[1].FragPos;
    dNormal = gs_in[1].Normal;
    dTexCoords = gs_in[1].TexCoords;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    dFragPos = gs_in[2].FragPos;
    dNormal = gs_in[2].Normal;
    dTexCoords = gs_in[2].TexCoords;
    EmitVertex();
    */
}