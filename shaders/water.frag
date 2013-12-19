#version 150

in vec3 vWorldPosition;
in vec3 vViewPosition;

out vec4 outColor;

uniform mat4 modelView;
uniform float time;
uniform vec3 playerPosition;
uniform vec3 fogColor;
uniform float viewDistance;

const vec3 lightAmbient  = vec3(1.0, 1.0, 1.0);
const vec3 lightDiffuse  = vec3(1.0, 1.0, 1.0);
const vec3 lightSpecular = vec3(1.0, 1.0, 1.0);

const vec3 materialAmbient  = vec3( 70.0,  80.0, 190.0) / 255.0;
const vec3 materialDiffuse  = vec3(100.0, 110.0, 230.0) / 255.0;
const vec3 materialSpecular = vec3(255.0, 248.0, 207.0) / 255.0;
const float materialShininess = 10.0;

vec3 perturbedNormal() {
    vec3 normal = vec3(0.0, 1.0, 0.0);

    // todo: perturb the normal...

    return normalize(normalize(modelView*vec4(normal, 0.0)).xyz);
}

void main() {
    vec3 color;

    // day-night cycle
    float t = time * 0.1;
    vec3 lightDirection = normalize(normalize(modelView*vec4(sin(t), cos(t), 0.0, 0.0)).xyz);


    // phong shading
    vec3 normal = perturbedNormal();
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
    

    outColor = vec4(color, 0.8);
}