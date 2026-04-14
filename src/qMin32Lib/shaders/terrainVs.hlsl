float4x4 g_mWorld;
float4x4 g_mView;
float4x4 g_mProj;
float4x4 g_mTex0Transform;
float4x4 g_mTex1Transform;

struct VS_IN
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
};

struct VS_OUT
{
    float4 Position : POSITION0;
    float2 UV0 : TEXCOORD0;
    float2 UV1 : TEXCOORD1;
    float FogDist : TEXCOORD2;
    float3 WorldPos : TEXCOORD3;
    float3 Normal : TEXCOORD4;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.Position, 1.0f), g_mWorld);
    float4 viewPos = mul(worldPos, g_mView);

    output.Position = mul(viewPos, g_mProj);

    float4 uv0 = mul(float4(viewPos.xyz, 1.0f), g_mTex0Transform);
    float4 uv1 = mul(float4(viewPos.xyz, 1.0f), g_mTex1Transform);

    output.UV0 = uv0.xy;
    output.UV1 = uv1.xy;
    output.FogDist = -viewPos.z;
    output.WorldPos = input.Position; 
    output.Normal = input.Normal;

    return output;
}