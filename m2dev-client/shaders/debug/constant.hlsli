// in this file we will keep all constant struct that will be used in our shaders
// this will make it easier to maintain and update our shader code

struct Light
{
    int Type;
    float4 Diffuse;
    float4 Specular;
    float4 Ambient;

    float3 Position;
    float3 Direction;
    float Range;

    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;

    float Theta;
    float Phi;
};
Light gLight;

