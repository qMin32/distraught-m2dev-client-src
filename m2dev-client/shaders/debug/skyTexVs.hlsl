float4x4 g_mWorld;
float4x4 g_mView;
float4x4 g_mProj;

struct VS_INPUT
{
    float3 vPos : POSITION;
    float4 vDiffuse : COLOR0;
    float2 vTex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 vPos : POSITION;
    float2 vTex0 : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 vWorldPos = mul(float4(input.vPos, 1.0f), g_mWorld);
    float4 vViewPos = mul(vWorldPos, g_mView);
    output.vPos = mul(vViewPos, g_mProj);
    output.vTex0 = input.vTex;
    return output;
}