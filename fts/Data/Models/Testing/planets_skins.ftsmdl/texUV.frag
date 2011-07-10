#version 130
precision highp float;
precision lowp int;

smooth in vec2 TexCo0;

uniform vec4 uMaterialDiffuse;

out vec4 oColor;

void main ()
{
#ifdef UNIFORM_DIFFUSE_COLOR
    oColor = mix(uMaterialDiffuse, vec4(TexCo0.st, 0.0, 1.0), 0.5);
#else
    oColor = vec4(TexCo0.st, 0.0, 1.0);
#endif
}
