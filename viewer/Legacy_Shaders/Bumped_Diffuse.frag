#version 130
varying vec4 gl_TexCoord[gl_MaxTextureCoords];
uniform sampler2D mainTexture;
uniform sampler2D normalTexture;
uniform vec3 lightColor, lightDirection;
uniform mat4 osg_ViewMatrix;

varying vec4 eyeVec;
varying vec3 normalVec, tangentVec, binormalVec;

void main()
{
    vec2 uv = gl_TexCoord[0].st;
    vec4 color = texture(mainTexture, uv);
    vec4 normal = texture(normalTexture, uv);
    
    vec3 lightDir = normalize(mat3(osg_ViewMatrix) * lightDirection);
    float diff = max(0.0, dot(normalVec.xyz, lightDir));
    gl_FragColor.rgb = color.rgb * lightColor * diff;
    gl_FragColor.a = color.a;
}

// Legacy Shaders/Bumped Specular
// Legacy Shaders/Reflective/Bumped Specular
// Legacy Shaders/Transparent/Diffuse
// Legacy Shaders/Transparent/Cutout/Diffuse
// Legacy Shaders/Transparent/Bumped Diffuse
// Legacy Shaders/Transparent/Cutout/Bumped Specular
// Legacy Shaders/Reflective/Bumped Diffuse
