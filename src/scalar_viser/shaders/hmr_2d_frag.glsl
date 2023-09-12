#version 130 core

uniform sampler3D volTex;
uniform sampler1D colTblTex;

uniform float minHeight;
uniform float maxHeight;
uniform float height;

in vec3 vertex;

void main() {
    vec3 pos = vertex;
    pos = (1.f + pos) * .5f;
    pos.z = (height - minHeight) / (maxHeight - minHeight);

    float scalar = texture(volTex, pos).r;
    vec4 col = texture(colTblTex, scalar);

    gl_FragColor.rgb = col.rgb;
    gl_FragColor.a = 1.f;
}
