#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#include "Common.hlsl"

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
    float4 SsaoPosH : POSITION1;
    float4 ShadowPosH : POSITION2;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;

    ObjectData objData = gObjectBuffer[gCameraData.ObjectBufferOffset + gObjectBufferIndex];

    float4 posW = mul(float4(vin.PosL, 1.0f), objData.World);
    vout.PosW = posW.xyz;
    vout.PosH = mul(posW, gCameraData.ViewProj);

    vout.NormalW = mul(vin.NormalL, (float3x3) objData.World);
    vout.TangentW = mul(vin.TangentU, (float3x3) objData.World);

    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), objData.TexTransform);
    vout.TexC = mul(texC, gMaterialData[objData.MaterialIndex].MatTransform).xy;

    vout.SsaoPosH = mul(posW, gCameraData.ViewProjTex);
    vout.ShadowPosH = mul(posW, gCameraData.ShadowTransform);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    ObjectData objData = gObjectBuffer[gCameraData.ObjectBufferOffset + gObjectBufferIndex];
    MaterialData matData = gMaterialData[objData.MaterialIndex];

    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    float shininess = 1.0f - roughness;

    pin.NormalW = normalize(pin.NormalW);

    float3 toEyeW = normalize(gCameraData.EyePosW - pin.PosW);

    float4 ambientAccess = 1.0f;
    float ambientFactor = AmbientMap.Sample(gsamLinearClamp, pin.SsaoPosH.xy / pin.SsaoPosH.w).r;
    float4 ambient = ambientFactor * gCameraData.AmbientLight * diffuseAlbedo;

    Material mat = { diffuseAlbedo, fresnelR0, shininess };

    float3 shadowFactor = 1.0f;
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);

    float4 directLight = ComputeLighting(
        gCameraData.Lights,
        mat,
        pin.PosW,
        pin.NormalW,
        toEyeW,
        shadowFactor
    );

    float4 litColor = ambient + directLight;

    float3 r = reflect(-toEyeW, pin.NormalW);
    float4 reflectionColor = SkyMap.Sample(gsamLinearWrap, r);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, pin.NormalW, r);

    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    litColor.a = diffuseAlbedo.a;

    return litColor;
}
