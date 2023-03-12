struct pixelShaderInput
{
    float4 projectionPosition : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(pixelShaderInput input) : SV_TARGET
{
    return input.color;
}