#version 130
precision highp float;
precision lowp int;

out vec4 oColor;

uniform vec3 uMaterialDiffuse = vec3(1.0);

void main ()
{
    oColor = vec4(uMaterialDiffuse, 1.0);
}
