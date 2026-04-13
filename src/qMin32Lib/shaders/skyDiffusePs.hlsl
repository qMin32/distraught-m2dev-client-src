struct PS_INPUT
{
    float4 vDiffuse : COLOR0;
};

float4 main(PS_INPUT input) : COLOR0
{
    return input.vDiffuse;
}