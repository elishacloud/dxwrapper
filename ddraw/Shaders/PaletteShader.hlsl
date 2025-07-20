sampler2D SurfaceTex : register(s0);
sampler2D PaletteTex : register(s1); // 256x1 palette texture

float4 main(float2 texCoords : TEXCOORD) : COLOR
{
    // Assume input is normalized index in [0,1], map to texel center
    float index = tex2D(SurfaceTex, texCoords).r * 255.0;
    float2 paletteUV = float2((index + 0.5) / 256.0, 0.0);
    return tex2D(PaletteTex, paletteUV);
}