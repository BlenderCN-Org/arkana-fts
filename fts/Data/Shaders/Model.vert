#version 130
precision highp float;
precision lowp int;

in vec3 aVertexPosition;

#include "/RigDeform.vertinc"

#ifdef D_LIT_OPTION
#  include "/Lighting.vertinc"
#endif

#ifdef D_TEXTURED_OPTION
#  include "/Texturing.vertinc"
#endif

// Don't call this uNormalMatrix, because for whatever reason, on nVidia
// drivers, this will cause the uBoneRotation uniform above to be garbage
// in Arkana-FTS sourcecode.
// Everything that is called "uN" followed by whatever causes this bug.
// Probably some specific "Optimization" by the nvidia drivers ....
// I am serious, I searched for this during days and narrowed it down to that.
#ifdef D_LIT_OPTION
    uniform mat3 qNormalMatrix = mat3(1.0);
    uniform mat4 uModelViewMatrix = mat4(1.0);

    in vec3 aVertexNormal;
    smooth out vec3 Normal;
#endif

uniform mat4 uModelViewProjectionMatrix = mat4(1.0);

invariant gl_Position;

void main()
{
    // Transform the vertices in model-space.
    vec3 localPos = deform(aVertexPosition);

#ifdef D_LIT_OPTION
    vec3 localNor = deformInvTran(aVertexNormal);

    Normal = qNormalMatrix * localNor;
    vec3 pos = (uModelViewMatrix * vec4(localPos, 1.0)).xyz;

    doLightingCalculus(qNormalMatrix, pos);
#endif

#ifdef D_TEXTURED_OPTION
    doTexturingCalculus();
#endif

    gl_Position = uModelViewProjectionMatrix * vec4(localPos, 1.0);
}
