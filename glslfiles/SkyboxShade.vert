#version 150
in vec3 in_Position;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    gl_Position =   projection * view * vec4(in_Position, 1.0);  
    TexCoords = in_Position;
}  