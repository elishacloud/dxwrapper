float4 g_offset          : register(c1);   // Pixel offset (usually -0.5, -0.5, 0, 0)

struct VS
{
    float4 pos : POSITION;
    float fog : FOG;
    float4 color[2] : COLOR;
    float4 tex[8] : TEXCOORD;
};

// D3DFVF_XYZ	    POSITION	float3 pos : POSITION;
// D3DFVF_XYZRHW	POSITION	float4 pos : POSITION;
// D3DFVF_NORMAL	NORMAL	    float3 normal : NORMAL;
// D3DFVF_DIFFUSE	COLOR0	    float4 color[0] : COLOR0;
// D3DFVF_SPECULAR	COLOR1	    float4 color[1] : COLOR1;
// D3DFVF_TEX1	    TEXCOORD0	float2 tex[0] : TEXCOORD0;
// D3DFVF_TEX2	    ...	        float2 tex[1] : TEXCOORD1; 

VS main(VS i)
{
    VS o = i;

    // Apply pixel center offset (usually -0.5 on X and Y)
    o.pos = i.pos + g_offset;

    // Clamp Z to [0.0, 1.0]
    o.pos.z = saturate(i.pos.z);

    return o;
}