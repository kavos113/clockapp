#include "App.h"

#include <array>
#include <chrono>
#include <format>

#define MAIN_TIMER 1

App::App()
    : m_hwnd(nullptr)
{
    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WindowProc,
        .hInstance = GetModuleHandle(nullptr),
        .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = reinterpret_cast<HBRUSH>((COLOR_WINDOW + 1)),
        .lpszClassName = className,
    };

    RegisterClassEx(&wc);
}

App::~App()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    UnregisterClass(className, GetModuleHandle(nullptr));
}

int App::createWindow(int width, int height)
{
    HWND hwnd = CreateWindowEx(
        0,
        className,
        L"Hello, World!",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );
    if (hwnd == nullptr)
    {
        return -1;
    }

    RECT windowRect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &windowRect, 0);

    constexpr int margin = 20;
    int x = windowRect.right - width - margin;
    int y = windowRect.top + 20;
    SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    return 0;
}

void App::initD2D()
{
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        m_d2dFactory.GetAddressOf()
    );
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to create D2D factory", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    std::array featureLevels = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext;
    hr = D3D11CreateDevice(
        nullptr, // Use default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, // No software device
        creationFlags,
        featureLevels.data(), static_cast<UINT>(featureLevels.size()),
        D3D11_SDK_VERSION,
        &d3dDevice,
        nullptr, // Feature level not needed
        &d3dContext
    );
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to create D3D11 device", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    hr = d3dDevice.As(&dxgiDevice);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to get IDXGIDevice", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    hr = m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to create D2D device", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    hr = m_d2dDevice->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
        &m_d2dContext
    );
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to create D2D device context", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
        .Width = size.width,
        .Height = size.height,
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
        .Stereo = false,
        .SampleDesc = { 1, 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2, // Double buffering
        .Scaling = DXGI_SCALING_NONE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
        .Flags = 0
    };

    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to get DXGI adapter", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
    hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to get DXGI factory", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    hr = dxgiFactory->CreateSwapChainForHwnd(
        d3dDevice.Get(),
        m_hwnd,
        &swapChainDesc,
        nullptr, // No additional swap chain options
        nullptr, // No restrict to output
        &m_swapChain
    );
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to create swap chain", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    createSurfaceBitmap();

    hr = m_d2dContext->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Red),
        &m_brush
    );
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to create D2D solid color brush", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    m_dwriteEngine = std::make_unique<DWriteEngine>(m_d2dContext, rc);
}

void App::run()
{
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    SetTimer(m_hwnd, MAIN_TIMER, 1000, nullptr);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT App::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    App *app = nullptr;

    if (uMsg == WM_CREATE)
    {
        auto *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
        app = static_cast<App*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));

        app->m_hwnd = hwnd;
    }
    else
    {
        app = reinterpret_cast<App*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (app)
    {
        return app->handleMessage(uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT App::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        onPaint();
        return 0;

    case WM_SIZE:
        onResize();
        return 0;

    case WM_TIMER:
        if (wParam == MAIN_TIMER)
        {
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
}

void App::onPaint()
{
    PAINTSTRUCT ps;
    BeginPaint(m_hwnd, &ps);

    m_d2dContext->BeginDraw();
    m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

    m_dwriteEngine->draw(createDrawInfo());

    if (FAILED(m_d2dContext->EndDraw()))
    {
        m_d2dContext.Reset();
        m_brush.Reset();
    }

    m_swapChain->Present(1, 0);

    EndPaint(m_hwnd, &ps);
}

void App::onResize()
{
    if (!m_d2dContext)
    {
        return;
    }

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    m_d2dContext->SetTarget(nullptr);
    m_bitmap.Reset();

    HRESULT hr = m_swapChain->ResizeBuffers(
        2, // Double buffering
        size.width, size.height,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        0 // No additional flags
    );
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to resize swap chain buffers", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    createSurfaceBitmap();

    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void App::createSurfaceBitmap()
{
    Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to get DXGI surface", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    hr = m_d2dContext->CreateBitmapFromDxgiSurface(
        dxgiSurface.Get(),
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.0f, // Default DPI
            96.0f  // Default DPI
        ),
        &m_bitmap
    );
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to create D2D bitmap from DXGI surface", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    m_d2dContext->SetTarget(m_bitmap.Get());
}

DrawInfo App::createDrawInfo()
{
    using namespace std::chrono;

    auto now = zoned_time{
        current_zone(),
        floor<seconds>(system_clock::now())
    };

    std::wstring timeString = std::format(L"{:%H:%M:%S}", now);

    return DrawInfo{
        .timeString = timeString
    };
}
