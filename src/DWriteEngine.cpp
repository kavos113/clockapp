#include "DWriteEngine.h"

#include <iostream>
#include <stdexcept>

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
        FONT_FAMILY_TIMER.c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        FONT_SIZE_TIMER,
        LOCALE.c_str(),
        &m_formatTimer
    );
    if (FAILED(hr))
    {
        std::cerr << "Failed to create text format" << std::endl;
        throw std::runtime_error("failed to create text format");
    }

    hr = m_formatTimer->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    if (FAILED(hr))
    {
        std::cerr << "Failed to set text alignment" << std::endl;
        throw std::runtime_error("failed to set text alignment");
    }

    hr = m_formatTimer->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    if (FAILED(hr))
    {
        std::cerr << "Failed to set paragraph alignment" << std::endl;
        throw std::runtime_error("failed to set paragraph alignment");
    }

    hr = m_factory->CreateTextFormat(
        FONT_FAMILY_OTHERS.c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        FONT_SIZE_OTHERS,
        LOCALE.c_str(),
        &m_formatCPU
    );
    if (FAILED(hr))
    {
        std::cerr << "Failed to create text format for cpu" << std::endl;
        throw std::runtime_error("failed to create text format");
    }

    hr = m_factory->CreateTextFormat(
        FONT_FAMILY_OTHERS.c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        FONT_SIZE_OTHERS,
        LOCALE.c_str(),
        &m_formatMemory
    );
    if (FAILED(hr))
    {
        std::cerr << "Failed to create text format for memory" << std::endl;
        throw std::runtime_error("failed to create text format");
    }

    hr = m_factory->CreateTextFormat(
        FONT_FAMILY_OTHERS.c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        FONT_SIZE_OTHERS,
        LOCALE.c_str(),
        &m_formatNetwork
    );
    if (FAILED(hr))
    {
        std::cerr << "Failed to create text format for network" << std::endl;
        throw std::runtime_error("failed to create text format");
    }

    hr = m_renderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black),
        &m_blackBrush
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
        m_formatTimer.Get(),
        D2D1::RectF(0, 0, static_cast<FLOAT>(m_rect.right - m_rect.left), static_cast<FLOAT>(m_rect.bottom - m_rect.top)),
        m_blackBrush.Get()
    );

    constexpr float pfWidth = 280.0f;
    float pfHeight = FONT_SIZE_OTHERS * 1.2f;
    float pfHeightBase = FONT_SIZE_TIMER * 1.3f;
    float pfLeft = static_cast<float>(m_rect.right - m_rect.left) - pfWidth;
    float pfRight = pfLeft + pfWidth;

    m_renderTarget->DrawText(
        info.cpuUsage.c_str(),
        info.cpuUsage.size(),
        m_formatCPU.Get(),
        D2D1::RectF(pfLeft, pfHeightBase, pfRight, pfHeightBase + pfHeight),
        m_blackBrush.Get()
    );

    m_renderTarget->DrawText(
        info.memoryUsage.c_str(),
        info.memoryUsage.size(),
        m_formatMemory.Get(),
        D2D1::RectF(pfLeft, pfHeightBase + pfHeight, pfRight, pfHeightBase + pfHeight * 2),
        m_blackBrush.Get()
    );

    m_renderTarget->DrawText(
        info.networkUsage.c_str(),
        info.networkUsage.size(),
        m_formatNetwork.Get(),
        D2D1::RectF(pfLeft, pfHeightBase + pfHeight * 2, pfRight, pfHeightBase + pfHeight * 3),
        m_blackBrush.Get()
    );
}
