sampler2D TextureSampler : register(s0); // Declare the sampler
float4 ColorKeyLow : register(c0);       // Declare the color key low constant
float4 ColorKeyHigh : register(c1);      // Declare the color key high constant

float4 main(float2 texCoords : TEXCOORD) : COLOR
{
	// Sample the texture
	float4 texColor = tex2D(TextureSampler, texCoords);

	// Check if the texel color is within the color key range
	if (all(texColor.rgb >= ColorKeyLow.rgb && texColor.rgb <= ColorKeyHigh.rgb))
	{
	    // Discard pixels within the color key range
	    discard;
	}

	// Output the texture color
	return texColor;
}