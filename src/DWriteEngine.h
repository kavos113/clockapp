#ifndef SRC_DWRITEENGINE_H
#define SRC_DWRITEENGINE_H

#include <wrl/client.h>
#include <d2d1_1.h>
#include <dwrite.h>

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
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_format;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush;
};


#endif //SRC_DWRITEENGINE_H