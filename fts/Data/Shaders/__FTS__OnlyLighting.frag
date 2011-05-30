#version 130
precision highp float;
precision lowp int;

smooth in vec3 EyeVector;
smooth in vec3 Normal;
smooth in vec3 LightDir;

out vec4 oColor;

#include "/BasicLighting.shadinc"

void main ()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(LightDir);
    vec3 E = normalize(EyeVector);

    vec3 lightCol = getLightingColor(L, N, E);

    oColor = vec4(lightCol, 1.0);
}
