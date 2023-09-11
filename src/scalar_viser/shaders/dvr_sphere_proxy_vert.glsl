#version 130 core

out vec4 vertex;

void main() {
    vertex = gl_Vertex;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
