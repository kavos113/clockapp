#include "DWriteEngine.h"

#include <iostream>
#include <stdexcept>
#include <string>

DWriteEngine::DWriteEngine(const Microsoft::WRL::ComPtr<ID2D1RenderTarget>& target, RECT rc)
    : m_renderTarget(target), m_rect(rc)
{
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        &m_factory
    );
    if (FAILED(hr))
    {
        std::cerr << "failed to create dwrite factory" << std::endl;
        throw std::runtime_error("failed to create dwrite factory");
    }

    hr = m_factory->CreateTextFormat(
        L"Rounded Mplus 1c",
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        128.0f,
        L"ja-JP",
        &m_format
    );
    if (FAILED(hr))
    {
        std::cerr << "Failed to create text format" << std::endl;
        throw std::runtime_error("failed to create text format");
    }

    hr = m_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    if (FAILED(hr))
    {
        std::cerr << "Failed to set text alignment" << std::endl;
        throw std::runtime_error("failed to set text alignment");
    }

    hr = m_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    if (FAILED(hr))
    {
        std::cerr << "Failed to set paragraph alignment" << std::endl;
        throw std::runtime_error("failed to set paragraph alignment");
    }

    hr = m_renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black),
        &m_brush
    );
    if (FAILED(hr))
    {
        std::cerr << "failed to create solid color brush" << std::endl;
        throw std::runtime_error("failed to create solid color brush");
    }
}

void DWriteEngine::draw(const DrawInfo& info) const
{
    m_renderTarget->DrawText(
        info.timeString.c_str(),
        info.timeString.size(),
        m_format.Get(),
        D2D1::RectF(0, 0, static_cast<FLOAT>(m_rect.right - m_rect.left), static_cast<FLOAT>(m_rect.bottom - m_rect.top)),
        m_brush.Get()
    );
}
