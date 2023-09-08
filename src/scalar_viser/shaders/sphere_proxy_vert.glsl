#version 130 core

out vec4 vertex;
out vec2 texCoord;

void main() {
    vertex = gl_Vertex;
    texCoord = gl_MultiTexCoord0.st;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
