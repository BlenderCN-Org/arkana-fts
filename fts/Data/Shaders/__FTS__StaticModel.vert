#version 130
precision highp float;
precision lowp int;

in vec3 aVertexPosition;
in vec3 aVertexNormal;
in vec2 aVertexTexCo0;

uniform mat3 uNormalMatrix = mat3(1.0);
uniform mat4 uModelViewMatrix = mat4(1.0);
uniform mat4 uModelViewProjectionMatrix = mat4(1.0);
uniform vec3 uLightDirection = vec3(0.0, 0.0, 1.0);

smooth out vec3 EyeVector;
smooth out vec3 Normal;
smooth out vec3 LightDir;
smooth out vec2 TexCo0;

invariant gl_Position;

void main()
{
    Normal = uNormalMatrix * aVertexNormal;
// For completely flat shading
//    LightDir = uNormalMatrix * aVertexNormal;
// For diffuse shading
    LightDir = uNormalMatrix * uLightDirection;
    vec3 pos = vec3(uModelViewMatrix * vec4(aVertexPosition, 1.0));

    EyeVector = -pos;
    TexCo0 = aVertexTexCo0;

    gl_Position = uModelViewProjectionMatrix * vec4(aVertexPosition, 1.0);
}
