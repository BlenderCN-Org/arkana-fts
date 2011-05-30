#version 130
precision highp float;
precision lowp int;

smooth in vec3 EyeVector;
smooth in vec3 Normal;
smooth in vec3 LightDir;
smooth in vec2 TexCo0;

uniform vec3 uLightDiffuse = vec3(1.0), uMaterialDiffuse = vec3(1.0);
uniform vec4 uPlayerColor = vec4(0.0, 1.0, 1.0, 1.0);

out vec4 oColor;

void main ()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(LightDir);
    float nDotL = max(dot(N, L), 0.0);

    oColor = vec4(TexCo0.st, 0.0, 1.0);
}
