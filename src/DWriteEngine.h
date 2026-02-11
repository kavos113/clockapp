#ifndef SRC_DWRITEENGINE_H
#define SRC_DWRITEENGINE_H

#include <wrl/client.h>
#include <d2d1_1.h>
#include <dwrite.h>

#include <string>

#include "DrawInfo.h"

class DWriteEngine
{
public:
    DWriteEngine(const Microsoft::WRL::ComPtr<ID2D1RenderTarget>& target, RECT rc);
    ~DWriteEngine() = default;

    void draw(const DrawInfo& info) const;

private:
    Microsoft::WRL::ComPtr<ID2D1RenderTarget> m_renderTarget;
    RECT m_rect;

    Microsoft::WRL::ComPtr<IDWriteFactory> m_factory;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_blackBrush;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_formatTimer;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_formatCPU;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_formatMemory;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_formatNetwork;

    const std::wstring FONT_FAMILY_TIMER = L"Rounded Mplus 1c";
    const std::wstring FONT_FAMILY_OTHERS = L"Consolas";
    const std::wstring LOCALE = L"ja-JP";
    const float FONT_SIZE_TIMER = 128.0f;
    const float FONT_SIZE_OTHERS = 32.0f;
};


#endif //SRC_DWRITEENGINE_H