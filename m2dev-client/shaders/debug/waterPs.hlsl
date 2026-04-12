sampler2D g_texWaterNormal : register(s0);

float g_fFogNear;
float g_fFogFar;
float4 g_vFogColor;
float4 g_vWaterColor;

struct PS_INPUT
{
    float4 vDiffuse : COLOR0;
    float2 vTex0 : TEXCOORD0;
    float2 vTex1 : TEXCOORD1;
    float fFog : TEXCOORD2;
};

float4 main(PS_INPUT input) : COLOR0
{
    float3 vNormal1 = tex2D(g_texWaterNormal, input.vTex0).rgb * 2.0f - 1.0f;
    float3 vNormal2 = tex2D(g_texWaterNormal, input.vTex1).rgb * 2.0f - 1.0f;

    float3 vNormal = normalize(vNormal1 + vNormal2);
    
    float fDistort = (vNormal.x + vNormal.y) * 0.5f;

    float4 vColor = g_vWaterColor;
    vColor.rgb += fDistort;
    vColor.rgb = saturate(vColor.rgb);

    vColor.a = g_vWaterColor.a * input.vDiffuse.a;
    vColor *= float4(0.3f, 0.6f, 0.7f, 1.0f);
    
    float fFogFactor = saturate((input.fFog - g_fFogNear) / (g_fFogFar - g_fFogNear));
    vColor.rgb = lerp(vColor.rgb, g_vFogColor.rgb, fFogFactor);

    return vColor;
}