#version 130 core

uniform float minHeight;
uniform float height;

out vec3 vertex;

void main() {
    vec4 pos = gl_Vertex;

    float scale = height / minHeight;
    pos.xyz = scale * pos.xyz;

    vertex = pos.xyz;
    gl_Position = gl_ModelViewProjectionMatrix * pos;
}
