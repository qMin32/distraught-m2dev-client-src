row_major float4x4 g_mWorld;
row_major float4x4 g_mView;
row_major float4x4 g_mProj;
row_major float4x4 g_mLightViewProj;

struct VS_INPUT
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float2 TexCoord2 : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 Position : POSITION0;
    float2 TexCoord : TEXCOORD0;
    float2 TexCoord2 : TEXCOORD1;
    float3 WorldNormal : TEXCOORD2;
    float3 WorldPos : TEXCOORD3;
    float4 Diffuse : COLOR0;
    float2 SphereUV : TEXCOORD4;
    float4 ShadowPos : TEXCOORD5;
};

bool g_bRenderingShadowMap;

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 worldPos = mul(float4(input.Position, 1.0f), g_mWorld);
    output.WorldPos = worldPos.xyz;
    
    output.ShadowPos = mul(worldPos, g_mLightViewProj);
    
    if (g_bRenderingShadowMap)
        output.Position = output.ShadowPos;
    else
    {
        float4 viewPos = mul(worldPos, g_mView);
        output.Position = mul(viewPos, g_mProj);
    }
    
    output.TexCoord = input.TexCoord;
    output.TexCoord2 = input.TexCoord2;
    output.WorldNormal = normalize(mul(input.Normal, (float3x3) g_mWorld));
    output.Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float3 normalCS = mul(output.WorldNormal, (float3x3) g_mView);
    output.SphereUV = normalCS.xy * 1.0f + 1.0f;
    
    return output;
}