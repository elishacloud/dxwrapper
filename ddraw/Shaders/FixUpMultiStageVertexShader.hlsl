float4 offset     : register(c0);
float4 multiplier : register(c1);
int texCount      : register(c2);

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

    o.tex[1] = texCount > 1 ? o.tex[1] : o.tex[0];
    o.tex[2] = texCount > 2 ? o.tex[2] : o.tex[0];
    o.tex[3] = texCount > 3 ? o.tex[3] : o.tex[0];
    o.tex[4] = texCount > 4 ? o.tex[4] : o.tex[0];
    o.tex[5] = texCount > 5 ? o.tex[5] : o.tex[0];
    o.tex[6] = texCount > 6 ? o.tex[6] : o.tex[0];
    o.tex[7] = texCount > 7 ? o.tex[7] : o.tex[0];

    return o;
}