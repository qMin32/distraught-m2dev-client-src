#include "constant.hlsli"

sampler2D DiffuseSampler : register(s0);
sampler2D AlphaSampler : register(s1);

float4 g_vFogColor;
float g_fFogNear;
float g_fFogFar;

struct VS_OUT
{
    float4 Position : POSITION0;
    float2 UV0 : TEXCOORD0;
    float2 UV1 : TEXCOORD1;
    float FogDist : TEXCOORD2;
    float3 WorldPos : TEXCOORD3;
    float3 Normal : TEXCOORD4;
};

float4 main(VS_OUT input) : COLOR0
{
    float3 n = normalize(input.Normal);
    float3 blend = abs(n);
    blend = pow(blend, 1.0f);
    blend /= (blend.x + blend.y + blend.z + 0.0001f);

    float texScale = 0.0024f;
    float4 cx = tex2D(DiffuseSampler, input.WorldPos.yz * texScale);
    float4 cy = tex2D(DiffuseSampler, input.WorldPos.xz * texScale);
    float4 cz = tex2D(DiffuseSampler, input.WorldPos.xy * texScale);
    float4 diffuse = cx * blend.x + cy * blend.y + cz * blend.z;

    float alpha = tex2D(AlphaSampler, input.UV1).a;

    float3 lightDir;
    float attenuation = 1.0f;
    
    if (gLight.Type == 1)
    {
        float3 toLight = gLight.Position.xyz - input.WorldPos;
        float dist = length(toLight);
        lightDir = normalize(toLight);
        attenuation = 1.0f / (gLight.Attenuation0 + gLight.Attenuation1 * dist + gLight.Attenuation2 * dist * dist);
        attenuation *= saturate(1.0f - dist / gLight.Range);
    }
    else if (gLight.Type == 2) 
    {
        float3 toLight = gLight.Position.xyz - input.WorldPos;
        float dist = length(toLight);
        lightDir = normalize(toLight);
        attenuation = 1.0f / (gLight.Attenuation0 + gLight.Attenuation1 * dist + gLight.Attenuation2 * dist * dist);
        attenuation *= saturate(1.0f - dist / gLight.Range);

        float3 spotDir = normalize(-gLight.Direction);
        float spotCos = dot(lightDir, spotDir);
        float innerCos = cos(gLight.Theta * 0.5f);
        float outerCos = cos(gLight.Phi * 0.5f);
        float spotFactor = saturate((spotCos - outerCos) / max(innerCos - outerCos, 0.0001f));
        attenuation *= pow(spotFactor, gLight.Falloff);
    }
    else 
    {
        lightDir = normalize(-gLight.Direction);
        attenuation = 1.0f;
    }

    float NdotL = saturate(dot(n, lightDir));

    float3 ambient = gLight.Ambient.rgb * diffuse.rgb;
    float skycolor = 0.3f;
    float3 sky = float3(skycolor, skycolor, skycolor) * diffuse.rgb;
    float3 lit = gLight.Diffuse.rgb * diffuse.rgb * NdotL * attenuation;

    float3 color = ambient + sky + lit;

    float fogDistance = 1.3f;
    float fogFactor = saturate((g_fFogFar * fogDistance - input.FogDist) / (g_fFogFar * fogDistance - g_fFogNear));
    fogFactor = pow(fogFactor, 0.5f);

    float3 finalRgb = lerp(g_vFogColor.rgb, color, fogFactor);
    return float4(finalRgb, alpha);
}
