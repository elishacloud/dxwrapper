float4 offset     : register(c0);
float4 multiplier : register(c1);

struct VS
{
    float4 pos      : POSITION;
    float fog       : FOG;
    float4 color[2] : COLOR;
    float4 tex[8]   : TEXCOORD;
};

VS main(const VS i)
{
    const float max_rhw = 1U << 31;
    const float min_rhw = 1.0f / max_rhw;

    const float rhw = clamp(i.pos.w, min_rhw, max_rhw);
    const float w = 1.0f / rhw;

    VS o = i;

    o.pos = (i.pos + offset) * multiplier;

    o.pos.z = saturate(o.pos.z);

    o.pos.xyz *= w;
    o.pos.w = w;

    o.fog = i.color[1].a;

    return o;
}