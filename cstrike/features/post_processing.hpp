#pragma once

// WIP

#define IMGUI_DEFINE_MATH_OPERATORS
#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <imgui.h>

#include <algorithm>
#include <exception>
#include <memory>

#include "../../sdk/shaders/build/gaussian_blur.hpp"

struct VSConstants {
  DirectX::XMMATRIX projection;
};

namespace PostProcessing {
void setDevice(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
               ID3D11RenderTargetView* renderTarget,
               IDXGISwapChain* swapChain) noexcept;
void clearBlurTextures() noexcept;
void onDeviceReset() noexcept;
void performFullscreenBlur(ImDrawList* drawList, float alpha) noexcept;
void performBlur(ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max,
                 float alpha) noexcept;
}  // namespace PostProcessing
