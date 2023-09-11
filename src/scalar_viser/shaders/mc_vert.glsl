#version 130 core

uniform float minLatitute;
uniform float maxLatitute;
uniform float minLongtitute;
uniform float maxLongtitute;
uniform float minHeight;
uniform float maxHeight;

out vec3 vertex;
out vec3 normal;

void main() {
    float hDlt = maxHeight - minHeight;
    float latDlt = maxLatitute - minLatitute;
    float lonDlt = maxLongtitute - minLongtitute;

    float lat = minLatitute + latDlt * gl_Vertex.y;
    float lon = minLongtitute + lonDlt * gl_Vertex.x;
    float h = minHeight + hDlt * gl_Vertex.z;

    vec4 pos;
    pos.z = h * sin(lat);
    float r = h * cos(lat);
    pos.x = r * cos(lon);
    pos.y = r * sin(lon);
    pos.w = 1.f;
    //vec4 pos = gl_Vertex;

    vertex = gl_Vertex.xyz;
    normal = gl_Normal;

	gl_Position = gl_ModelViewProjectionMatrix * pos;
}
