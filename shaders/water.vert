#version 150

in vec3 position;

out vec3 vWorldPosition;
out vec3 vViewPosition;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec3 playerPosition;

void main() {
    vWorldPosition = position;
    vWorldPosition.xz += floor(playerPosition.xz / 2.0) * 2.0;
    vViewPosition = (modelView*vec4(vWorldPosition, 1.0)).xyz;

    gl_Position = projection*modelView*vec4(vWorldPosition, 1.0);
}