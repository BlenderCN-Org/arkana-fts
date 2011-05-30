#version 130
precision highp float;
precision lowp int;

smooth in vec3 EyeVector;
smooth in vec3 Normal;
smooth in vec3 LightDir;
smooth in vec2 TexCo0;

uniform sampler2D uTexture0;

out vec4 oColor;

#define D_HAS_PLAYER_COLOR_UNIFORM
uniform vec4 uPlayerColor = vec4(1.0);
#include "/Texturing.shadinc"

#include "/BasicLighting.shadinc"

void main ()
{
    vec3 texCol = getTextureColor(uTexture0, TexCo0.st);

    vec3 N = normalize(Normal);
    vec3 L = normalize(LightDir);
    vec3 E = normalize(EyeVector);

    vec3 litCol = getLightingColor(L, N, E);

    oColor = vec4(litCol * texCol, 1.0);
}
