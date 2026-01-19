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
    float4 PosView : SV_POSITION;
    float4 ShadowPosH : POSITION0;
    float4 SsaoPosH : POSITION1;
    float3 PosW : POSITION2;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexC : TEXCOORD;
};


VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;

    MaterialData matData = materialData[objectBuffer.materialIndex];

    float4 posW = mul(float4(vin.PosL, 1.0f), objectBuffer.World);
    vout.PosW = posW.xyz;

    vout.NormalW = mul(vin.NormalL, (float3x3)objectBuffer.World);

    vout.PosView = mul(posW, worldBuffer.ViewProj);

    vout.TangentW = mul(vin.TangentU, (float3x3)objectBuffer.World);

    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), objectBuffer.TexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;

    // Generate projective tex-coords to project SSAO map onto scene.
    vout.SsaoPosH = mul(posW, worldBuffer.ViewProjTex);


    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(posW, worldBuffer.ShadowTransform);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    MaterialData matData = materialData[objectBuffer.materialIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    uint diffuseMapIndex = matData.DiffuseMapIndex;
    uint normalMapIndex = matData.NormalMapIndex;

    diffuseAlbedo *= texturesMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);

#ifdef ALPHA_TEST
	clip(diffuseAlbedo.a - 0.3f);
#endif

    pin.NormalW = normalize(pin.NormalW);

    float4 normalMapSample = texturesMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);


    //bumpedNormalW = pin.NormalW;


    float3 toEyeW = normalize(worldBuffer.EyePosW - pin.PosW);

    pin.SsaoPosH /= pin.SsaoPosH.w;
    float ambientAccess = ssaoMap.Sample(gsamLinearClamp, pin.SsaoPosH.xy, 0.0f).r;


    float4 ambient = ambientAccess * worldBuffer.AmbientLight * diffuseAlbedo;


    // Only the first light casts a shadow.
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);


    const float shininess = (1.0f - roughness) * normalMapSample.a;
    Material mat = {diffuseAlbedo, fresnelR0, shininess};
    float4 directLight = ComputeLighting(worldBuffer.Lights, mat, pin.PosW,
                                         bumpedNormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;


    if (matData.MatPad1 != 0)
    {
    // Вектор из точки на поверхности к камере
        float3 V = toEyeW;

    // Направление отражения для кубмапы
        float3 R = reflect(-V, bumpedNormalW);

    // Цвет окружения из SkyMap (у тебя SkyMap = TextureCube register(t0) уже объявлен в Common.hlsl)
        float3 envColor = SkyMap.Sample(gsamLinearWrap, R).rgb;

    // Fresnel по Шлику: для "взгляда" используем V (а не R)
        float3 F = SchlickFresnel(fresnelR0, bumpedNormalW, V);

    // shininess у тебя уже вычислен (зависит от roughness и alpha normal map)
        litColor.rgb += envColor * F * shininess;
    }

    litColor.a = diffuseAlbedo.a;
    return litColor;
}
