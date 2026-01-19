#pragma once

#include "d3dUtil.h"
#include "GDescriptor.h"
#include "GTexture.h"

using namespace PEPEngine;
using namespace Graphics;
using namespace Allocator;
using namespace Utils;

class CubeMapRenderTarget
{
public:
    CubeMapRenderTarget(const std::shared_ptr<GDevice>& device, UINT size, DXGI_FORMAT format, DXGI_FORMAT depthFormat);

    void OnResize(UINT newSize);
    void BuildSRV(GDescriptor* srvHeap, UINT srvIndex);

    UINT GetSize() const;

    GTexture& GetCubeMap();
    GTexture& GetDepthMap();

    GDescriptor* GetRTV();
    GDescriptor* GetDSV();

    const D3D12_VIEWPORT& GetViewport() const;
    const D3D12_RECT& GetScissorRect() const;

    UINT GetSrvIndex() const;

private:
    void BuildResources();
    void BuildDescriptors();

    std::shared_ptr<GDevice> device;

    UINT size = 0;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;

    GTexture cubeMap;
    GTexture depthMap;

    GDescriptor rtvMemory;
    GDescriptor dsvMemory;

    D3D12_VIEWPORT viewport{};
    D3D12_RECT scissorRect{};

    UINT srvIndex = UINT_MAX;
};
