#include "post_processing.hpp"

// WIP

#define SAFE_RELEASE(p) \
  {                     \
    if (p) {            \
      (p)->Release();   \
      (p) = nullptr;    \
    }                   \
  }

static inline ID3D11Device* pDevice;  // DO NOT RELEASE!
static inline ID3D11DeviceContext* pDeviceContext;
static inline ID3D11RenderTargetView* pRenderTarget;  // DO NOT RELEASE!
static inline IDXGISwapChain* pSwapChain;

[[nodiscard]] static ID3D11Texture2D* createTexture(int width,
                                                    int height) noexcept {
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  ID3D11Texture2D* texture = nullptr;
  pDevice->CreateTexture2D(&desc, nullptr, &texture);
  return texture;
}

static void copyBackbufferToTexture(ID3D11Texture2D* texture) noexcept {
  pSwapChain->GetBuffer(0, __uuidof(texture),
                        reinterpret_cast<void**>(&texture));
}

static void setRenderTarget(ID3D11Texture2D* rtTexture) noexcept {
  ID3D11RenderTargetView* rtv = nullptr;
  pDevice->CreateRenderTargetView(rtTexture, nullptr, &rtv);
  pDeviceContext->OMSetRenderTargets(1, &rtv, nullptr);
  rtv->Release();
}

class ShaderProgram {
 public:
  ~ShaderProgram() {
    if (pixelShader) pixelShader->Release();
  }

  void use(float uniform, int location) const noexcept {
    pDeviceContext->PSSetShader(pixelShader, nullptr, 0);
    pDeviceContext->PSSetConstantBuffers(location, 1, &constBuffer);
  }

  void init(const BYTE* pixelShaderSrc, size_t size) noexcept {
    if (initialized) return;
    initialized = true;
    pDevice->CreatePixelShader(pixelShaderSrc, size, nullptr, &pixelShader);
  }

 private:
  ID3D11PixelShader* pixelShader = nullptr;
  ID3D11Buffer* constBuffer = nullptr;
  bool initialized = false;
};

class BlurEffect {
 public:
  static void draw(ImDrawList* drawList, float alpha) noexcept {
    instance()._draw(drawList, alpha);
  }

  static void draws(ImDrawList* drawList, const ImVec2& p_min,
                    const ImVec2& p_max, float alpha) noexcept {
    instance()._draws(drawList, p_min, p_max, alpha);
  }

  static void clearTextures() noexcept {
    SAFE_RELEASE(instance().pBlurTexture1);
    SAFE_RELEASE(instance().pBlurTexture2);
  }

 private:
  ID3D11RenderTargetView* pRTBackup = nullptr;
  ID3D11Texture2D* pBlurTexture1 = nullptr;
  ID3D11Texture2D* pBlurTexture2 = nullptr;
  ID3D11RasterizerState* pRasterStateWithScissorDisabled = nullptr;
  ID3D11RasterizerState* pOriginalRasterState = nullptr;
  ID3D11RenderTargetView* pBlurTexture1RTV = nullptr;
  ID3D11RenderTargetView* pBlurTexture2RTV = nullptr;
  ID3D11Buffer* pVSConstantBuffer = nullptr;

  ID3D11ShaderResourceView* pBlurTexture1SRV = nullptr;
  ID3D11ShaderResourceView* pBlurTexture2SRV = nullptr;

  ShaderProgram blurShaderX;
  ShaderProgram blurShaderY;

  int backbufferWidth = 0;
  int backbufferHeight = 0;
  static constexpr auto blurDownsample = 2;

  BlurEffect() = default;
  BlurEffect(const BlurEffect&) = delete;

  ~BlurEffect() {
    SAFE_RELEASE(pRTBackup);
    SAFE_RELEASE(pRasterStateWithScissorDisabled);
    SAFE_RELEASE(pOriginalRasterState);
    SAFE_RELEASE(pBlurTexture1);
    SAFE_RELEASE(pBlurTexture2);
    SAFE_RELEASE(pBlurTexture1RTV);
    SAFE_RELEASE(pBlurTexture2RTV);
    SAFE_RELEASE(pBlurTexture1SRV);
    SAFE_RELEASE(pBlurTexture2SRV);
    SAFE_RELEASE(pVSConstantBuffer);
  }

  static BlurEffect& instance() noexcept {
    static BlurEffect blurEffect;
    return blurEffect;
  }

  static void begin(const ImDrawList*, const ImDrawCmd*) noexcept {
    instance()._begin();
  }

  static void firstPass(const ImDrawList*, const ImDrawCmd*) noexcept {
    instance()._firstPass();
  }

  static void secondPass(const ImDrawList*, const ImDrawCmd*) noexcept {
    instance()._secondPass();
  }

  static void end(const ImDrawList*, const ImDrawCmd*) noexcept {
    instance()._end();
  }

  void createTextures() noexcept {
    if (const auto [width, height] = ImGui::GetIO().DisplaySize;
        backbufferWidth != static_cast<int>(width) ||
        backbufferHeight != static_cast<int>(height)) {
      clearTextures();
      backbufferWidth = static_cast<int>(width);
      backbufferHeight = static_cast<int>(height);
    }

    if (!pBlurTexture1)
      pBlurTexture1 = createTexture(backbufferWidth / blurDownsample,
                                    backbufferHeight / blurDownsample);
    if (!pBlurTexture2)
      pBlurTexture2 = createTexture(backbufferWidth / blurDownsample,
                                    backbufferHeight / blurDownsample);

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format =
        DXGI_FORMAT_R8G8B8A8_UNORM;  // Match this with your texture format.
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    pDevice->CreateRenderTargetView(pBlurTexture1, &rtvDesc, &pBlurTexture1RTV);
    pDevice->CreateRenderTargetView(pBlurTexture2, &rtvDesc, &pBlurTexture2RTV);
  }

  void createShaders() noexcept {
    blurShaderX.init(reinterpret_cast<const BYTE*>(gaussian_blur_x),
                     sizeof(gaussian_blur_x));
    blurShaderY.init(reinterpret_cast<const BYTE*>(gaussian_blur_y),
                     sizeof(gaussian_blur_y));
  }

  void _begin() noexcept {
    pDeviceContext->OMGetRenderTargets(1, &pRTBackup, nullptr);

    copyBackbufferToTexture(pBlurTexture1);

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ID3D11SamplerState* samplerState;
    pDevice->CreateSamplerState(&sampDesc, &samplerState);
    pDeviceContext->PSSetSamplers(0, 1, &samplerState);

    pDeviceContext->RSGetState(&pOriginalRasterState);

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;  // Solid fill mode
    rasterDesc.CullMode = D3D11_CULL_BACK;   // Cull back-facing triangles
    rasterDesc.ScissorEnable = FALSE;        // Disable scissor test
    rasterDesc.DepthClipEnable = TRUE;       // Enable depth clipping
    HRESULT hr = pDevice->CreateRasterizerState(
        &rasterDesc, &pRasterStateWithScissorDisabled);

    if (FAILED(hr)) {
      printf("Failed CreateRasterizerState\n");
    }

    pDeviceContext->RSSetState(pRasterStateWithScissorDisabled);

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(VSConstants);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    pDevice->CreateBuffer(&cbDesc, nullptr, &pVSConstantBuffer);

    float aspectRatio = static_cast<float>(backbufferWidth) /
                        static_cast<float>(backbufferHeight);
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(
        -1.0f / aspectRatio, 1.0f / aspectRatio, -1.0f, 1.0f, 0.0f, 1.0f);

    VSConstants vsConsts = {};
    vsConsts.projection = XMMatrixTranspose(
        projection);  // HLSL expects matrices to be transposed

    pDeviceContext->UpdateSubresource(pVSConstantBuffer, 0, nullptr, &vsConsts,
                                      0, 0);
    pDeviceContext->VSSetConstantBuffers(0, 1, &pVSConstantBuffer);
  }

  void _firstPass() noexcept {
    blurShaderX.use(2.0f / (backbufferWidth / blurDownsample), 0);
    pDeviceContext->OMSetRenderTargets(1, &pBlurTexture2RTV, nullptr);
  }

  void _secondPass() noexcept {
    blurShaderY.use(2.0f / (backbufferHeight / blurDownsample), 0);
    pDeviceContext->OMSetRenderTargets(1, &pBlurTexture1RTV, nullptr);

    pDevice->CreateShaderResourceView(pBlurTexture2, nullptr,
                                      &pBlurTexture2SRV);
  }

  void _end() noexcept {
    pDeviceContext->OMSetRenderTargets(1, &pRTBackup, nullptr);
    pRTBackup->Release();
    pDeviceContext->PSSetShader(nullptr, nullptr, 0);
    pDeviceContext->RSSetState(pOriginalRasterState);

    if (pRasterStateWithScissorDisabled)
      pRasterStateWithScissorDisabled->Release();
  }

  void _draw(ImDrawList* drawList, float alpha) noexcept {
    createTextures();
    createShaders();

    if (!pBlurTexture1RTV || !pBlurTexture2RTV) return;

    drawList->AddCallback(&begin, nullptr);
    for (int i = 0; i < 8; ++i) {
      drawList->AddCallback(&firstPass, nullptr);
      pDevice->CreateShaderResourceView(pBlurTexture1, nullptr,
                                        &pBlurTexture1SRV);
      drawList->AddImage(
          (ImTextureID)pBlurTexture1SRV, ImVec2(-1.0f, -1.0f),
          ImVec2(1.0f, 1.0f));  // Assuming blurTexture1SRV is the Shader
                                // Resource View for blurTexture1
      drawList->AddCallback(&secondPass, nullptr);
      drawList->AddImage((ImTextureID)pBlurTexture2SRV, ImVec2(-1.0f, -1.0f),
                         ImVec2(1.0f, 1.0f));  // And similarly for blurTexture2
    }
    drawList->AddCallback(&end, nullptr);
    drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

    drawList->AddImage(
        reinterpret_cast<ImTextureID>(pBlurTexture1SRV), {0.0f, 0.0f},
        {backbufferWidth * 1.0f, backbufferHeight * 1.0f}, {0.0f, 0.0f},
        {1.0f, 1.0f}, IM_COL32(255, 255, 255, 255 * alpha));

    if (pOriginalRasterState) pOriginalRasterState->Release();
  }

  void _draws(ImDrawList* drawList, const ImVec2& p_min, const ImVec2& p_max,
              float alpha) noexcept {
    createTextures();
    createShaders();

    if (!pBlurTexture1RTV || !pBlurTexture2RTV) return;

    drawList->AddCallback(&begin, nullptr);
    for (int i = 0; i < 8; ++i) {
      drawList->AddCallback(&firstPass, nullptr);
      pDevice->CreateShaderResourceView(pBlurTexture1, nullptr,
                                        &pBlurTexture1SRV);
      drawList->AddImage(reinterpret_cast<ImTextureID>(pBlurTexture1SRV), p_min,
                         p_max, {1.0f, 1.0f});
      drawList->AddCallback(&secondPass, nullptr);
      pDevice->CreateShaderResourceView(pBlurTexture2, nullptr,
                                        &pBlurTexture2SRV);
      drawList->AddImage(reinterpret_cast<ImTextureID>(pBlurTexture2SRV), p_min,
                         p_max, {1.0f, 1.0f});
    }
    drawList->AddCallback(&end, nullptr);
    drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

    drawList->AddImage(
        reinterpret_cast<ImTextureID>(pBlurTexture2SRV), p_min, p_max,
        ImVec2((float)backbufferWidth * 1, (float)backbufferHeight * 1),
        ImVec2((float)backbufferWidth * 1, (float)backbufferHeight * 1),
        IM_COL32(255, 255, 255, 255 * alpha));
  }
};

void PostProcessing::setDevice(ID3D11Device* device,
                               ID3D11DeviceContext* deviceContext,
                               ID3D11RenderTargetView* renderTarget,
                               IDXGISwapChain* swapChain) noexcept {
  pDevice = device;
  pDeviceContext = deviceContext;
  pRenderTarget = renderTarget;
  pSwapChain = swapChain;
}

void PostProcessing::clearBlurTextures() noexcept {
  BlurEffect::clearTextures();
}

void PostProcessing::onDeviceReset() noexcept { BlurEffect::clearTextures(); }

void PostProcessing::performFullscreenBlur(ImDrawList* drawList,
                                           float alpha) noexcept {
  BlurEffect::draw(drawList, alpha);
}

void PostProcessing::performBlur(ImDrawList* drawList, const ImVec2& p_min,
                                 const ImVec2& p_max, float alpha) noexcept {
  BlurEffect::draws(drawList, p_min, p_max, alpha);
}
