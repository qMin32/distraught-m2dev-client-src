#include "constant.hlsli"

bool g_bHasOpacity;
bool g_bSpecularEnable;
float g_fSpecularPower;
float4 g_vCameraPos;

sampler2D g_texDiffuse : register(s0);
sampler2D g_texOpacity : register(s1);

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

float4 main(VS_OUTPUT input) : COLOR0
{
    float3 N = normalize(input.WorldNormal);

    float4 albedo1 = tex2D(g_texDiffuse, input.TexCoord);
    float4 albedo2 = tex2D(g_texDiffuse, input.TexCoord2);
    float4 albedo = albedo1 * albedo2;

    float3 lightDir = normalize(-gLight.Direction);

    float NdotL = saturate(dot(N, lightDir));

    float3 ambient = gLight.Ambient.rgb * albedo.rgb;
    float skycolorvalue = 0.6f;
    float3 skylight = float3(skycolorvalue, skycolorvalue, skycolorvalue) * albedo.rgb;
    float3 diffuse = gLight.Diffuse.rgb * albedo.rgb * NdotL;
    
    float3 specularCol = 0.0f;
    if (g_bSpecularEnable)
    {
        specularCol = albedo.a * g_fSpecularPower;
    }

    float3 color = ambient + skylight + (diffuse + specularCol);
    float fAlpha = albedo.a * input.Diffuse.a;

    return float4(color, fAlpha);
}
