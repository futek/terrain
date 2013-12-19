#version 150

in vec3 position;

out vec3 vWorldPosition;
out vec3 vWorldTangent;
out vec3 vWorldBinormal;
out vec3 vWorldNormal;
out vec3 vViewPosition;
out vec3 vViewTangent;
out vec3 vViewBinormal;
out vec3 vViewNormal;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec3 playerPosition;

// inspired by http://www.iquilezles.org/www/articles/morenoise/morenoise.htm

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

vec3 noised(vec2 x) {
    vec2 p = floor(x);
    vec2 f = fract(x);

    float n = p.x + p.y*57.0;

    float a = hash(n +  0.0);
    float b = hash(n +  1.0);
    float c = hash(n + 57.0);
    float d = hash(n + 58.0);

    float k0 = a;
    float k1 = b - a;
    float k2 = c - a;
    float k3 = a - b - c + d;

    vec2 u = ((6.0*f - 15.0)*f + 10.0)*f*f*f;
    vec2 du = ((30.0*f - 60.0)*f + 30.0)*f*f;

    return vec3(
        k0 + k1*u.x + k2*u.y + k3*u.x*u.y,
        du * vec2(k1 + k3*u.y, k2 + k3*u.x)
    );
}

vec3 terrain(vec2 p) {
    float w = 0.5;
    float f = 0.0;
    vec2 d = vec2(0.0);
    
    for (int i = 0; i < 6; i++) {
        vec3 n = noised(p);
        d += n.yz;
        f += w * n.x / (1.0 + dot(d, d));
        w *= 0.5;
        p *= 2.0;
    }

    return vec3(f, d);
}

vec3 terrain2(vec2 p) {
    return vec3(cos(p.x)*sin(p.y), -sin(p.x)*sin(p.y), cos(p.x)*cos(p.y));
}

void main() {
    // floor -> avoid wiggling
    // 2.0 -> jump 2 vertices at a time to retain same mesh geometry
    vec3 p = position.xyz + floor(playerPosition / 2.0) * 2.0;

    float scale = 200.0;
    float delta = -50.0;

    vec3 t = terrain(p.xz / scale);
    float height = t.x * scale + delta; // height
    vec2 d = t.yz; // derivatives

    vWorldPosition = vec3(p.x, height, p.z);
    vWorldTangent  = normalize(vec3(0.0, d.y, 1.0));
    vWorldBinormal =  normalize(vec3(1.0, d.x, 0.0));
    vWorldNormal   = cross(vWorldTangent, vWorldBinormal);

    vViewPosition  = (modelView * vec4(vWorldPosition, 1.0)).xyz;
    vViewTangent   = (modelView * vec4(vWorldTangent, 0.0)).xyz;
    vViewBinormal  = (modelView * vec4(vWorldBinormal, 0.0)).xyz;
    vViewNormal    = (modelView * vec4(vWorldNormal, 0.0)).xyz;

    gl_Position = projection * modelView * vec4(vWorldPosition, 1.0);
}