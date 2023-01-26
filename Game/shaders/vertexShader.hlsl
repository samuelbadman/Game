struct vertexShaderInput
{
	float3 position : POSITION;
	//float3 normal : NORMAL;
	//float4 color : COLOR_ZERO;
	//float2 uv : TEXCOORD_ZERO;
};

struct vertexShaderOutput
{
	float4 finalPosition : SV_POSITION;
	//float3 normal : NORMAL;
	//float4 color : COLOR_ZERO;
	//float2 uv : TEXCOORD_ZERO;
};

vertexShaderOutput main(vertexShaderInput input)
{
	vertexShaderOutput output;
	output.finalPosition = float4(input.position, 1.0f);
	//output.normal = input.normal;
	//output.color = input.color;
	//output.uv = input.uv;

	return output;
}