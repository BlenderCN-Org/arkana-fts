#version 130
precision highp float;
precision lowp int;

in vec3 aVertexPosition;
in vec3 aVertexNormal;
in vec2 aVertexTextureCo;

uniform mat4 uModelViewProjectionMatrix = mat4(1.0);

smooth out vec2 TexCo0;

invariant gl_Position;

void main()
{
    vec3 finalVertPos = aVertexPosition + (noise1(3.1415)+1.0)*0.75*abs(aVertexNormal.x)*aVertexNormal;
    TexCo0 = aVertexTextureCo;
    gl_Position = uModelViewProjectionMatrix * vec4(finalVertPos, 1.0);
}
