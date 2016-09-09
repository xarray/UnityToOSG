varying vec4 gl_TexCoord[gl_MaxTextureCoords];
uniform mat4 gl_TextureMatrix[gl_MaxTextureCoords];
varying vec4 eyeVec;
varying vec3 normalVec;

void main()
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    normalVec = normalize(vec3(gl_NormalMatrix * gl_Normal));
    eyeVec = gl_ModelViewMatrix * gl_Vertex;
}
