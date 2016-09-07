varying vec4 gl_TexCoord[gl_MaxTextureCoords];
uniform sampler2D mainTexture;
uniform sampler2D lightTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissionTexture;
uniform samplerCube reflectTexture;

void main()
{
    vec4 color = texture2D(mainTexture, gl_TexCoord[0].st);
    vec4 light = texture2D(lightTexture, gl_TexCoord[1].st);
    gl_FragColor = color * light;
}
