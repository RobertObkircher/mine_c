varying vec2 uv;

#ifdef COMPILING_VERTEX_SHADER

uniform mat4 MVP;

attribute vec3 vPos;
attribute vec2 vUV;

void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    uv = vUV;
}


#endif

#ifdef COMPILING_FRAGMENT_SHADER

uniform sampler2D textureSampler;

void main()
{
    gl_FragColor = texture( textureSampler, uv );
//    gl_FragColor = texture( textureSampler, uv ) + vec4(1.0, 0,0,1.0);
}

#endif
