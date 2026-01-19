#include "pch.h"
#include "CubeMapRenderTarget.h"

CubeMapRenderTarget::CubeMapRenderTarget(const std::shared_ptr<GDevice>& device, 
                                        UINT size, DXGI_FORMAT format, DXGI_FORMAT depthFormat)
    : device(device), size(size), format(format), depthFormat(depthFormat)
{
    viewport = { 0.0f, 0.0f, static_cast<float>(size), static_cast<float>(size), 0.0f, 1.0f };
    scissorRect = { 0, 0, static_cast<int>(size), static_cast<int>(size) };

    rtvMemory = this->device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 6);
    dsvMemory = this->device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

    BuildResources();
    BuildDescriptors();
}

void CubeMapRenderTarget::OnResize(UINT newSize)
{
    if (size == newSize) return;

    size = newSize;

    viewport = { 0.0f, 0.0f, static_cast<float>(size), static_cast<float>(size), 0.0f, 1.0f };
    scissorRect = { 0, 0, static_cast<int>(size), static_cast<int>(size) };

    BuildResources();
    BuildDescriptors();
}

void CubeMapRenderTarget::BuildSRV(GDescriptor* srvHeap, UINT srvIndex)
{
    this->srvIndex = srvIndex;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

    cubeMap.CreateShaderResourceView(&srvDesc, srvHeap, srvIndex);
}

UINT CubeMapRenderTarget::GetSize() const
{
    return size;
}

GTexture& CubeMapRenderTarget::GetCubeMap()
{
    return cubeMap;
}

GTexture& CubeMapRenderTarget::GetDepthMap()
{
    return depthMap;
}

GDescriptor* CubeMapRenderTarget::GetRTV()
{
    return &rtvMemory;
}

GDescriptor* CubeMapRenderTarget::GetDSV()
{
    return &dsvMemory;
}

const D3D12_VIEWPORT& CubeMapRenderTarget::GetViewport() const
{
    return viewport;
}

const D3D12_RECT& CubeMapRenderTarget::GetScissorRect() const
{
    return scissorRect;
}

UINT CubeMapRenderTarget::GetSrvIndex() const
{
    return srvIndex;
}

void CubeMapRenderTarget::BuildResources()
{
    D3D12_RESOURCE_DESC cubeDesc{};
    cubeDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    cubeDesc.Alignment = 0;
    cubeDesc.Width = size;
    cubeDesc.Height = size;
    cubeDesc.DepthOrArraySize = 6;
    cubeDesc.MipLevels = 1;
    cubeDesc.Format = format;
    cubeDesc.SampleDesc.Count = 1;
    cubeDesc.SampleDesc.Quality = 0;
    cubeDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    cubeDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    const float clear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    CD3DX12_CLEAR_VALUE clearColor(format, clear);

    cubeMap = GTexture(device, cubeDesc, L"DynamicCubeMap", TextureUsage::RenderTarget, &clearColor);

    D3D12_RESOURCE_DESC depthDesc{};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = size;
    depthDesc.Height = size;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = depthFormat;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearDepth{};
    clearDepth.Format = depthFormat;
    clearDepth.DepthStencil.Depth = 1.0f;
    clearDepth.DepthStencil.Stencil = 0;

    depthMap = GTexture(device, depthDesc, L"DynamicCubeDepth", TextureUsage::Depth, &clearDepth);
}

void CubeMapRenderTarget::BuildDescriptors()
{
    for (UINT i = 0; i < 6; ++i)
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = 0;
        rtvDesc.Texture2DArray.PlaneSlice = 0;
        rtvDesc.Texture2DArray.FirstArraySlice = i;
        rtvDesc.Texture2DArray.ArraySize = 1;

        cubeMap.CreateRenderTargetView(&rtvDesc, &rtvMemory, i);
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = depthFormat;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D.MipSlice = 0;

    depthMap.CreateDepthStencilView(&dsvDesc, &dsvMemory, 0);
}
