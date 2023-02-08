struct vertexShaderInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct pixelShaderInput
{
	float4 finalPosition : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

cbuffer objectConstants : register(b0, space0)
{
    float4x4 wvpMatrix;
}

pixelShaderInput main(vertexShaderInput input)
{
    pixelShaderInput output;
    output.finalPosition = mul(float4(input.position, 1.0f), wvpMatrix);
	output.normal = input.normal;
	output.color = input.color;
	output.uv = input.uv;

	return output;
}