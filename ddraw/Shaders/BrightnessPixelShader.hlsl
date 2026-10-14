float4 Adjustments : register(c0);

sampler2D inputTexture : register(s0);
sampler1D gammaRampTexture : register(s1);

float4 main(float2 texCoord : TEXCOORD0) : COLOR0 {
    float4 color = tex2D(inputTexture, texCoord);

    float contrast = Adjustments.x;
    float brightness = Adjustments.y;

    // Apply per-channel gamma correction using the ramp texture
    color.r = tex1D(gammaRampTexture, color.r).r;
    color.g = tex1D(gammaRampTexture, color.g).g;
    color.b = tex1D(gammaRampTexture, color.b).b;

    // Apply contrast & brightness
    color.rgb = ((color.rgb - 0.5) * contrast) + 0.5 + brightness;

    // Clamp final result
    color.rgb = saturate(color.rgb);

    return color;
}