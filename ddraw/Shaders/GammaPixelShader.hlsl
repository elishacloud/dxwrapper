sampler2D inputTexture : register(s0);
sampler1D gammaRampTexture : register(s1);

float4 main(float2 texCoord : TEXCOORD0) : COLOR0 {
    float4 color = tex2D(inputTexture, texCoord);

    // Apply per-channel gamma correction using the ramp texture
    color.r = tex1D(gammaRampTexture, color.r).r;
    color.g = tex1D(gammaRampTexture, color.g).g;
    color.b = tex1D(gammaRampTexture, color.b).b;

    return color;
}

technique GammaCorrection {
    pass P0 {
        PixelShader = compile ps_2_0 main();
    }
}