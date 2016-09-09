#version 130
varying vec4 gl_TexCoord[gl_MaxTextureCoords];
uniform sampler2D mainTexture;
uniform sampler2D lightTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissionTexture;
uniform samplerCube reflectTexture;

uniform vec3 lightColor, lightDirection;
uniform mat4 osg_ViewMatrix;

varying vec4 eyeVec;
varying vec3 normalVec, tangentVec, binormalVec;

void main()
{
    vec4 color = texture(mainTexture, gl_TexCoord[0].st);
    vec4 light = texture(lightTexture, gl_TexCoord[1].st);
    
    vec3 lightDir = normalize(mat3(osg_ViewMatrix) * lightDirection);
    float diff = max(0.0, dot(normalVec.xyz, lightDir));
    gl_FragColor.rgb = color.rgb * light.rgb * lightColor * diff;
    gl_FragColor.a = color.a;
}
