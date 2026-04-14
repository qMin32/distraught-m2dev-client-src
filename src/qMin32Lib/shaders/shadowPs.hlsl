float g_fFogNear;
float g_fFogFar;
float4 g_vFogColor;
int g_useDynamicShadow;

sampler2D StaticShadowSampler : register(s0);
sampler2D DynamicShadowSampler : register(s1);

float4 main(float2 suv : TEXCOORD0,
            float2 duv : TEXCOORD1,
            float fogDist : TEXCOORD2) : COLOR0
{
    
    float shadow = tex2D(StaticShadowSampler, suv).r;

    if (g_useDynamicShadow != 0)
    {
        float dynamicShadow = tex2D(DynamicShadowSampler, duv).r;
        float dynamicShadowStrength = 0.5f;
        shadow = lerp(shadow, shadow * dynamicShadow, dynamicShadowStrength);
    }

    float fogFactor = saturate((g_fFogFar - fogDist) / (g_fFogFar - g_fFogNear));
    shadow = lerp(1.0f, shadow, fogFactor);

    return float4(shadow, shadow, shadow, 1.0f);
}