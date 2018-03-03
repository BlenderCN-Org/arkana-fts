#version 130
precision highp float;
precision lowp int;

#ifdef D_LIT_OPTION
#  include "/Lighting.fraginc"
   smooth in vec3 Normal;
#else
   uniform vec3 uMaterialDiffuse = vec3(1.0, 0.0, 0.0);
#endif

#ifdef D_TEXTURED_OPTION
#  include "/Texturing.fraginc"
#endif

out vec4 oColor;

void main()
{
#ifdef D_LIT_OPTION
    vec3 N = normalize(Normal);
    vec3 litCol = getLightingColor(N);
#else
    vec3 litCol = uMaterialDiffuse;
#endif

#if defined(D_TEXTURED_OPTION)
    vec4 texCol = getTextureColor();
    oColor = vec4(litCol, 1.0) * texCol;
#else
    oColor = vec4(litCol, 1.0);
#endif
}
