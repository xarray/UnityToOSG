varying vec4 gl_TexCoord[gl_MaxTextureCoords];
uniform mat4 gl_TextureMatrix[gl_MaxTextureCoords];

void main()
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
}
