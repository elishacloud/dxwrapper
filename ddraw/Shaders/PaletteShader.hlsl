sampler2D SurfaceTex : register(s0);
sampler2D PaletteTex : register(s1);

float4 main(float2 texCoords : TEXCOORD) : COLOR
{
    float pIndex = tex2D(SurfaceTex, texCoords).r;
    return tex2D(PaletteTex, float2(pIndex * (255./256) + (0.5/256), 0));
}
