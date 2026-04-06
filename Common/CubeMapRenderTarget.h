#pragma once

#include <array>

#include "d3dUtil.h"
#include "GCommandList.h"
#include "GDescriptor.h"
#include "GTexture.h"

using namespace PEPEngine;
using namespace Graphics;
using namespace Allocator;
using namespace Utils;

class CubeMapRenderTarget
{
public:
    static constexpr UINT FaceCount = 6;

    CubeMapRenderTarget(const std::shared_ptr<GDevice>& device, UINT size, DXGI_FORMAT format, DXGI_FORMAT depthFormat);

    void OnResize(UINT newSize);
    void BuildSRV(GDescriptor* srvHeap, UINT srvIndex);
    void CopyToCubeMap(const std::shared_ptr<GCommandList>& cmdList);

    UINT GetSize() const;

    GTexture& GetCubeMap();
    GTexture& GetCubeMap(UINT faceIndex);
    std::array<GTexture, FaceCount>& GetCubeMaps();
    GTexture& GetDepthMap();

    GDescriptor* GetRTV(UINT faceIndex);
    GDescriptor* GetDSV();

    const D3D12_VIEWPORT& GetViewport() const;
    const D3D12_RECT& GetScissorRect() const;

    UINT GetSrvIndex(UINT faceIndex = 0) const;

private:
    void BuildResources();
    void BuildDescriptors();

    std::shared_ptr<GDevice> device;

    UINT size = 0;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;

    GTexture cubeMap;
    std::array<GTexture, FaceCount> cubeMaps;
    GTexture depthMap;

    std::array<GDescriptor, FaceCount> rtvMemory;
    GDescriptor dsvMemory;

    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissorRect{};

    UINT srvIndex = UINT_MAX;
};
