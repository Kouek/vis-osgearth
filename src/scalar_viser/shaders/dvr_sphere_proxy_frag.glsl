#version 130 core

uniform sampler3D volTex;
uniform sampler1D tfTex;

uniform vec3 eyePos;
uniform float dt;

uniform float minLatitute;
uniform float maxLatitute;
uniform float minLongtitute;
uniform float maxLongtitute;
uniform float minHeight;
uniform float maxHeight;

in vec4 vertex;

vec3 intersectOuterSphere(vec3 p) {
    vec3 p2eDir = normalize(eyePos - p);
    vec3 p2c = -p;

    float l = dot(p2eDir, p2c);
    float m2 = minHeight * minHeight - l * l;
    float q = sqrt(maxHeight * maxHeight - m2);
    return p + (q + l) * p2eDir;
}

struct Hit {
    int isHit;
    float tEntry;
};
Hit intersectInnerSphere(vec3 p, vec3 p2eDir) {
    Hit hit = Hit(0, 0.f);
    vec3 p2c = -p;

    float l = dot(p2eDir, p2c);
    if (l <= 0.f)
        return hit;
    float m2 = maxHeight * maxHeight - l * l;
    float innerR2 = minHeight * minHeight;
    if (m2 >= innerR2)
        return hit;
    float q = sqrt(innerR2 - m2);

    hit.isHit = 1;
    hit.tEntry = l - q;
    return hit;
}

float anotherIntersectionOuterSphere(vec3 p, vec3 p2eDir) {
    vec3 p2c = -p;
    float l = dot(p2eDir, p2c);
    return 2.f * l;
}

void main() {
//#define TEST
#ifdef TEST
    vec3 d = normalize(vertex.xyz - eyePos);
    vec3 pos = vertex.xyz;
    float r = sqrt(pos.x * pos.x + pos.y * pos.y);
    float lat = atan(pos.z / r);
    float lon = atan(pos.y, pos.x);
    int entryOutOfRng = 0;
    if (lat < minLatitute)
        entryOutOfRng |= 1;
    if (lat > maxLatitute)
        entryOutOfRng |= 2;
    if (lon < minLongtitute)
        entryOutOfRng |= 4;
    if (lon > maxLongtitute)
        entryOutOfRng |= 8;

    float tExit = anotherIntersectionOuterSphere(vertex.xyz, d);
    Hit hitInner = intersectInnerSphere(vertex.xyz, d);
    if (hitInner.isHit != 0)
        tExit = hitInner.tEntry;

    pos = vertex.xyz + tExit * d;
    r = sqrt(pos.x * pos.x + pos.y * pos.y);
    lat = atan(pos.z / r);
    lon = atan(pos.y, pos.x);
    if ((entryOutOfRng & 1) != 0 && lat < minLatitute)
        discard;
    if ((entryOutOfRng & 2) != 0 && lat > minLatitute)
        discard;
    if ((entryOutOfRng & 4) != 0 && lon < minLongtitute)
        discard;
    if ((entryOutOfRng & 8) != 0 && lon > minLongtitute)
        discard;

    float hDlt = maxHeight - minHeight;
    float latDlt = maxLatitute - minLatitute;
    float lonDlt = maxLongtitute - minLongtitute;

    pos = vertex.xyz;
    r = sqrt(pos.x * pos.x + pos.y * pos.y);
    lat = atan(pos.z / r);
    lon = atan(pos.y, pos.x);
    r = (r - minHeight) / hDlt;
    lat = (lat - minLatitute) / latDlt;
    lon = (lon - minLongtitute) / lonDlt;

    gl_FragColor = vec4(lon, lat, r, 1);
#else
    vec3 d = normalize(vertex.xyz - eyePos);
    vec3 pos = vertex.xyz;
    float r = sqrt(pos.x * pos.x + pos.y * pos.y);
    float lat = atan(pos.z / r);
    float lon = atan(pos.y, pos.x);
    int entryOutOfRng = 0;
    if (lat < minLatitute)
        entryOutOfRng |= 1;
    if (lat > maxLatitute)
        entryOutOfRng |= 2;
    if (lon < minLongtitute)
        entryOutOfRng |= 4;
    if (lon > maxLongtitute)
        entryOutOfRng |= 8;

    float tExit = anotherIntersectionOuterSphere(vertex.xyz, d);
    Hit hitInner = intersectInnerSphere(vertex.xyz, d);

    if (hitInner.isHit != 0)
        tExit = hitInner.tEntry;

    pos = vertex.xyz + tExit * d;
    r = sqrt(pos.x * pos.x + pos.y * pos.y);
    lat = atan(pos.z / r);
    lon = atan(pos.y, pos.x);
    if ((entryOutOfRng & 1) != 0 && lat < minLatitute)
        discard;
    if ((entryOutOfRng & 2) != 0 && lat > minLatitute)
        discard;
    if ((entryOutOfRng & 4) != 0 && lon < minLongtitute)
        discard;
    if ((entryOutOfRng & 8) != 0 && lon > minLongtitute)
        discard;

    float hDlt = maxHeight - minHeight;
    float latDlt = maxLatitute - minLatitute;
    float lonDlt = maxLongtitute - minLongtitute;

    vec4 color = vec4(0, 0, 0, 0);
    vec3 entry2Exit = pos - vertex.xyz;
    float tMax = length(entry2Exit);
    float tAcc = 0.f;
    pos.xyz = vertex.xyz;
    do {
        r = sqrt(pos.x * pos.x + pos.y * pos.y);
        lat = atan(pos.z / r);
        lon = atan(pos.y, pos.x);
        if (lat < minLatitute || lat > maxLatitute || lon < minLongtitute || lon > maxLongtitute) {
            pos += dt * d;
            tAcc += dt;
            continue;
        }

        r = (r - minHeight) / hDlt;
        lat = (lat - minLatitute) / latDlt;
        lon = (lon - minLongtitute) / lonDlt;

        float scalar = texture(volTex, vec3(lon, lat, r)).r;
        vec4 tfCol = texture(tfTex, scalar);

        color.rgb = color.rgb + (1.f - color.a) * tfCol.a * tfCol.rgb;
        color.a = color.a + (1.f - color.a) * tfCol.a;
        if (color.a > .95f)
            break;

        pos += dt * d;
        tAcc += dt;
    } while (tAcc < tMax);

    gl_FragColor = color;
#endif
}
