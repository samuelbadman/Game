struct vertexShaderInput
{
	float3 localPosition : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct pixelShaderInput
{
    float4 projectionPosition : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

cbuffer objectConstants : register(b0, space0)
{
    float4x4 wMatrix;
}

cbuffer cameraConstants : register(b1, space0)
{
    float4x4 vpMatrix;
}

pixelShaderInput main(vertexShaderInput input)
{
    float4 worldPosition = mul(float4(input.localPosition, 1.0f), wMatrix);
    float4 projectionPosition = mul(worldPosition, vpMatrix);

    pixelShaderInput output;
    output.projectionPosition = projectionPosition;
	output.normal = input.normal;
	output.color = input.color;
	output.uv = input.uv;

	return output;
}