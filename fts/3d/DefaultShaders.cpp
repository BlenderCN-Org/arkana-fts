const char* sErrorVert =
"#version 130\n"
"precision highp float;\n"
"precision lowp int;\n"
"\n"
"in vec3 aVertexPosition;\n"
"\n"
"uniform mat4 uModelViewProjectionMatrix;\n"
"\n"
"invariant gl_Position;\n"
"\n"
"void main ()\n"
"{\n"
"    gl_Position = uModelViewProjectionMatrix * vec4 (aVertexPosition, 1.0);\n"
"}\n";

const char* sErrorFrag =
"#version 130\n"
"precision highp float;\n"
"precision lowp int;\n"
"\n"
"out vec4 oColor;\n"
"\n"
"void main ()\n"
"{\n"
"    oColor = vec4 (1.0, 0.0, 0.0, 1.0);\n"
"}\n";
