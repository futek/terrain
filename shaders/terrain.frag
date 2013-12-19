#version 150

uniform mat4 modelView;
uniform float time;
uniform vec3 playerPosition;
uniform vec3 fogColor;
uniform float viewDistance;

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec3 vViewPosition;
in vec3 vViewNormal;

out vec4 outColor;

const vec3 lightAmbient  = vec3(1.0, 1.0, 1.0);
const vec3 lightDiffuse  = vec3(1.0, 1.0, 1.0);
const vec3 lightSpecular = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 color;

    // terrain material based on height
    vec3 materialAmbient  = vec3(0.0);
    vec3 materialDiffuse  = vec3(0.0);
    vec3 materialSpecular = vec3(0.0);
    float materialShininess = 100.0;

    float h = vWorldPosition.y;

    float slope = dot(vWorldNormal, vec3(0.0, 1.0, 0.0)); // vertical 0...1 horizontal
    h += slope * 10.0;

    if (h < 0.0) {
        color = vec3( 140.0, 157.0, 235.0) / 255.0; // water
    } else if (h < 6.0) {
        color = vec3(222.0, 219.0, 175.0) / 255.0; // sand
    } else if (h < 13.0) {
        color = vec3(185.0, 207.0, 147.0) / 255.0; // light foliage
    } else if (h < 58.0) {
        color = vec3( 61.0,  94.0,  57.0) / 255.0; // woods
    } else if (h < 78.0) {
        color = vec3(190.0, 191.0, 174.0) / 255.0; // rock
    } else {
        color = vec3(242.0, 244.0, 247.0) / 255.0; // snow        
    }

    materialAmbient  = color * 0.3;
    materialDiffuse  = color * 1.0;


    // day-night cycle
    float t = time * 0.1;
    vec3 lightDirection = normalize(normalize(modelView*vec4(sin(t), cos(t), 0.0, 0.0)).xyz);


    // phong shading
    //vec3 lightDirection = normalize(normalize(lightPosition - vec4(vViewPosition, 1.0)).xyz);
    vec3 normal = normalize(vViewNormal);
    vec3 viewDirection = normalize(-vViewPosition);
    vec3 reflection = reflect(-lightDirection, normal);

    vec3 ambient = lightAmbient*materialAmbient;

    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    float d = dot(lightDirection, normal);
    if (d > 0.0) diffuse = lightDiffuse*materialDiffuse*d;

    vec3 specular = vec3(0.0, 0.0, 0.0);
    d = dot(viewDirection, reflection);
    if (d > 0.0) specular = lightSpecular*materialSpecular*pow(d, materialShininess);

    color = ambient + diffuse + specular;


    // fog
    float dist = length(vViewPosition);
    color = mix(color, fogColor, min(dist*dist*dist / (viewDistance*viewDistance*viewDistance), 1.0));


    outColor = vec4(color, 1.0);
}