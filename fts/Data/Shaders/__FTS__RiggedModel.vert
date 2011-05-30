#version 130
precision highp float;
precision lowp int;

in vec3 aVertexPosition;
in vec3 aVertexNormal;
in vec2 aVertexTexCo0;
in vec4 aBoneIndicesAndWeights;

// Those initializers are the only ones that work.
// They init the rotations to the identity and the translations to 0.
uniform vec3 uBoneTranslation[20] = vec3[20](0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);
uniform mat3 uBoneRotation[20] = mat3[20](1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0);

// Don't call this uNormalMatrix, because for whatever reason, on nVidia
// drivers, this will cause the uBoneRotation uniform above to be garbage
// in Arkana-FTS sourcecode.
// Everything that is called "uN" followed by whatever causes this bug.
// Probably some specific "Optimization" by the nvidia drivers ....
// I am serious, I searched for this during days and narrowed it down to that.
uniform mat3 qNormalMatrix = mat3(1.0);
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
    vec3 localPos = vec3(0.0);
    vec3 localNor = vec3(0.0);
    for(int i = 0 ; i < 4 ; i++) {
        localPos += (uBoneRotation[int(aBoneIndicesAndWeights[i])] * aVertexPosition + uBoneTranslation[int(aBoneIndicesAndWeights[i])]) * fract(aBoneIndicesAndWeights[i]);
        localNor += (uBoneRotation[int(aBoneIndicesAndWeights[i])] * aVertexNormal) * fract(aBoneIndicesAndWeights[i]);
    }

    Normal = qNormalMatrix * localNor;
// For completely flat shading
//    LightDir = qNormalMatrix * aVertexNormal;
// For diffuse shading
    LightDir = qNormalMatrix * uLightDirection;
    vec3 pos = vec3(uModelViewMatrix * vec4(localPos, 1.0));

    EyeVector = -pos;
    TexCo0 = aVertexTexCo0;

    gl_Position = uModelViewProjectionMatrix * vec4(localPos, 1.0);
}
