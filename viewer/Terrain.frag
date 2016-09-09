#version 130
varying vec4 gl_TexCoord[gl_MaxTextureCoords];
uniform sampler2DArray splatTexture;
uniform sampler2D lightTexture;
uniform sampler2D alphaTexture;
uniform vec4 splatTilingOffsets[4];

uniform vec3 lightColor, lightDirection;
uniform mat4 osg_ViewMatrix;

varying vec4 eyeVec;
varying vec3 normalVec;

void main()
{
    ivec3 size = textureSize(splatTexture, 0);
    vec4 alpha = texture(alphaTexture, gl_TexCoord[0].st);
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 light = texture(lightTexture, gl_TexCoord[1].st);
    for ( int i=0; i<size.z; ++i )
    {
        vec4 offset = splatTilingOffsets[i];
        vec3 uv = vec3(gl_TexCoord[0].st * offset.xy + offset.zw, float(i));
        color += texture(splatTexture, uv) * alpha[3 - i];
    }
    
    vec3 lightDir = normalize(mat3(osg_ViewMatrix) * lightDirection);
    float diff = max(0.0, dot(normalVec.xyz, lightDir));
    gl_FragColor.rgb = color.rgb * light.rgb * lightColor * diff;
    gl_FragColor.a = color.a;
}
