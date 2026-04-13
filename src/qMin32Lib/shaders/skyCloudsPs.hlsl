sampler2D g_texCloud : register(s0);

struct PS_INPUT
{
    float4 vDiffuse : COLOR0;
    float2 vTex0 : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR0
{
    float4 vTex = tex2D(g_texCloud, input.vTex0);

    // amplifica albul norilor
    float3 vColor = vTex.rgb + input.vDiffuse.rgb * 0.5f;
    vColor = saturate(vColor);

    // alpha mai mare = mai opac
    float fAlpha = saturate(vTex.a * 2.0f);

    return float4(vColor, fAlpha);
}