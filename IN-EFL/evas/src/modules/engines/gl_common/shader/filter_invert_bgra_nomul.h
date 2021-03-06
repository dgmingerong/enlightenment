"#ifdef GL_ES\n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
"precision highp float;\n"
"#else\n"
"precision mediump float;\n"
"#endif\n"
"#endif\n"
"uniform sampler2D tex;\n"
"varying vec4 col;\n"
"varying vec2 tex_c;\n"
"void main()\n"
"{\n"
"   vec3 inv = vec3(1.0, 1.0, 1.0) - texture2D(tex,tex_c.xy).rgb;\n"
"   gl_FragColor = vec4(inv.r, inv.g, inv.b, 1.0);\n"
"}\n"
