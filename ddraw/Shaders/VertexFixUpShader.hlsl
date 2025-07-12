float4 g_FVFTypes        : register(c1);   // Flags: diffuse, specular, tex count
float4 g_offset          : register(c2);   // Pixel offset (only for RHW)

struct VS
{
    float4 pos : POSITION;
    float4 color[2] : COLOR;
    float2 tex[8] : TEXCOORD;
};

VS main(VS i)
{
    VS o = i;

    // Apply pixel center offset (usually -0.5 on X and Y)
    o.pos = i.pos + g_offset;

    // Clamp Z to [0.0, 1.0]
    o.pos.z = saturate(i.pos.z);

    // D3DFVF_XYZ	    POSITION	float3 pos : POSITION;
    // D3DFVF_XYZRHW	POSITION	float4 pos : POSITION;
    // D3DFVF_NORMAL	NORMAL	    float3 normal : NORMAL;
    // D3DFVF_DIFFUSE	COLOR0	    float4 color[0] : COLOR0;
    // D3DFVF_SPECULAR	COLOR1	    float4 color[1] : COLOR1;
    // D3DFVF_TEX1	    TEXCOORD0	float2 tex[0] : TEXCOORD0;
    // D3DFVF_TEX2	    ...	        float2 tex[1] : TEXCOORD1; 

    // Zero out unused values
    if (g_FVFTypes.y == 0.0f) o.color[0] = float4(0, 0, 0, 0);  // D3DFVF_DIFFUSE
    if (g_FVFTypes.z == 0.0f) o.color[1] = float4(0, 0, 0, 0);  // D3DFVF_SPECULAR

    if (g_FVFTypes.w < 1.0f) o.tex[0] = float2(0, 0);   // D3DFVF_TEX1
    if (g_FVFTypes.w < 2.0f) o.tex[1] = float2(0, 0);   // D3DFVF_TEX2
    if (g_FVFTypes.w < 3.0f) o.tex[2] = float2(0, 0);   // D3DFVF_TEX3
    if (g_FVFTypes.w < 4.0f) o.tex[3] = float2(0, 0);   // D3DFVF_TEX4
    if (g_FVFTypes.w < 5.0f) o.tex[4] = float2(0, 0);   // D3DFVF_TEX5
    if (g_FVFTypes.w < 6.0f) o.tex[5] = float2(0, 0);   // D3DFVF_TEX6
    if (g_FVFTypes.w < 7.0f) o.tex[6] = float2(0, 0);   // D3DFVF_TEX7
    if (g_FVFTypes.w < 8.0f) o.tex[7] = float2(0, 0);   // D3DFVF_TEX8

    return o;
}
