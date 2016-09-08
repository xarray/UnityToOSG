varying vec4 gl_TexCoord[gl_MaxTextureCoords];
uniform mat4 gl_TextureMatrix[gl_MaxTextureCoords];
attribute vec3 tangent;

varying vec4 eyeVec;
varying vec3 normalVec, tangentVec, binormalVec;

void main()
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
    normalVec = normalize(vec3(gl_NormalMatrix * gl_Normal));
    tangentVec = normalize(vec3(gl_NormalMatrix * tangent));
    binormalVec = cross(normalVec, tangentVec);
    eyeVec = gl_ModelViewMatrix * gl_Vertex;
}
