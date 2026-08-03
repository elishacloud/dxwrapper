struct VS
{
    float4 pos      : POSITION;   // XYZRHW
    float4 diffuse  : COLOR0;     // Diffuse
    float4 specular : COLOR1;     // Specular
    float2 tex0     : TEXCOORD0;  // First texture
};

VS main(VS v)
{
    const float max_rhw = 1U << 31;
    const float min_rhw = 1.0f / max_rhw;
    
    VS o;

    // Fix RHW and Z
    float rhw = v.pos.w;
    float z   = v.pos.z;

    if (!isfinite(rhw) || rhw == 0.0f)
    {
        rhw = 1.0f;
    }
    else
    {
        float tw = 1.0f / rhw;
        float tz = z * tw;

        tw = clamp(tw, min_rhw, max_rhw);

        z   = tz / tw;
        rhw = 1.0f / tw;
    }

    // Output
    o.pos      = float4(v.pos.x, v.pos.y, z, rhw);
    o.diffuse  = v.diffuse;
    o.specular = v.specular;
    o.tex0     = v.tex0;

    return o;
}