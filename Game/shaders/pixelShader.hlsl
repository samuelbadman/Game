struct pixelShaderOutput
{
	float4 finalPosition : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

float4 main(pixelShaderOutput input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}