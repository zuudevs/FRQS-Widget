#include "renderer_d2d.hpp"
#include <stdexcept>

namespace frqs::render {

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

RendererD2D::RendererD2D(platform::NativeHandle hwnd)
    : hwnd_(hwnd)
{
    if (!hwnd_) {
        throw std::runtime_error("Invalid window handle for renderer");
    }

    // Create D2D factory
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &factory_
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create Direct2D factory");
    }

    // Create DirectWrite factory
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&writeFactory_)
    );

    if (FAILED(hr)) {
        cleanup();
        throw std::runtime_error("Failed to create DirectWrite factory");
    }

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory_)
    );

    if (FAILED(hr)) {
        cleanup();
        throw std::runtime_error("Failed to create WIC factory");
    }

    // Create default text format
    hr = writeFactory_->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        14.0f,
        L"en-us",
        &defaultTextFormat_
    );

    if (FAILED(hr)) {
        cleanup();
        throw std::runtime_error("Failed to create default text format");
    }

    // Create render target
    if (!recreateDeviceResources()) {
        cleanup();
        throw std::runtime_error("Failed to create render target");
    }
}

RendererD2D::~RendererD2D() noexcept {
    cleanup();
}

// ============================================================================
// RENDER SESSION
// ============================================================================

void RendererD2D::beginRender() {
    if (!renderTarget_ || inRender_) return;

    inRender_ = true;
    renderTarget_->BeginDraw();
}

void RendererD2D::endRender() {
    if (!renderTarget_ || !inRender_) return;

    HRESULT hr = renderTarget_->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET) {
        recreateDeviceResources();
    }

    inRender_ = false;
}

// ============================================================================
// BASIC RENDERER INTERFACE
// ============================================================================

void RendererD2D::clear(const widget::Color& color) {
    if (!renderTarget_) return;

    renderTarget_->Clear(toD2DColor(color));
}

void RendererD2D::drawRect(
    const widget::Rect<int32_t, uint32_t>& rect,
    const widget::Color& color,
    float strokeWidth
) {
    if (!renderTarget_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    renderTarget_->DrawRectangle(
        toD2DRect(rect),
        brush,
        strokeWidth
    );

    brush->Release();
}

void RendererD2D::fillRect(
    const widget::Rect<int32_t, uint32_t>& rect,
    const widget::Color& color
) {
    if (!renderTarget_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    renderTarget_->FillRectangle(
        toD2DRect(rect),
        brush
    );

    brush->Release();
}

void RendererD2D::drawText(
    const std::wstring& text,
    const widget::Rect<int32_t, uint32_t>& rect,
    const widget::Color& color
) {
    if (!renderTarget_ || !defaultTextFormat_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    renderTarget_->DrawTextW(
        text.c_str(),
        static_cast<UINT32>(text.length()),
        defaultTextFormat_,
        toD2DRect(rect),
        brush
    );

    brush->Release();
}

void RendererD2D::pushClip(const widget::Rect<int32_t, uint32_t>& rect) {
    if (!renderTarget_) return;

    clipStack_.push(rect);
    renderTarget_->PushAxisAlignedClip(
        toD2DRect(rect),
        D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
    );
}

void RendererD2D::popClip() {
    if (!renderTarget_ || clipStack_.empty()) return;

    clipStack_.pop();
    renderTarget_->PopAxisAlignedClip();
}

// ============================================================================
// EXTENDED RENDERER INTERFACE
// ============================================================================

void RendererD2D::drawLine(
    const widget::Point<int32_t>& start,
    const widget::Point<int32_t>& end,
    const widget::Color& color,
    float strokeWidth
) {
    if (!renderTarget_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    renderTarget_->DrawLine(
        D2D1::Point2F(
            static_cast<float>(start.x),
            static_cast<float>(start.y)
        ),
        D2D1::Point2F(
            static_cast<float>(end.x),
            static_cast<float>(end.y)
        ),
        brush,
        strokeWidth
    );

    brush->Release();
}

void RendererD2D::drawEllipse(
    const widget::Rect<int32_t, uint32_t>& rect,
    const widget::Color& color,
    float strokeWidth
) {
    if (!renderTarget_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    D2D1_ELLIPSE ellipse;
    ellipse.point = D2D1::Point2F(
        static_cast<float>(rect.getCenterX()),
        static_cast<float>(rect.getCenterY())
    );
    ellipse.radiusX = rect.w / 2.0f;
    ellipse.radiusY = rect.h / 2.0f;

    renderTarget_->DrawEllipse(ellipse, brush, strokeWidth);
    brush->Release();
}

void RendererD2D::fillEllipse(
    const widget::Rect<int32_t, uint32_t>& rect,
    const widget::Color& color
) {
    if (!renderTarget_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    D2D1_ELLIPSE ellipse;
    ellipse.point = D2D1::Point2F(
        static_cast<float>(rect.getCenterX()),
        static_cast<float>(rect.getCenterY())
    );
    ellipse.radiusX = rect.w / 2.0f;
    ellipse.radiusY = rect.h / 2.0f;

    renderTarget_->FillEllipse(ellipse, brush);
    brush->Release();
}

void RendererD2D::drawRoundedRect(
    const widget::Rect<int32_t, uint32_t>& rect,
    float radiusX,
    float radiusY,
    const widget::Color& color,
    float strokeWidth
) {
    if (!renderTarget_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    D2D1_ROUNDED_RECT roundedRect;
    roundedRect.rect = toD2DRect(rect);
    roundedRect.radiusX = radiusX;
    roundedRect.radiusY = radiusY;

    renderTarget_->DrawRoundedRectangle(roundedRect, brush, strokeWidth);
    brush->Release();
}

void RendererD2D::fillRoundedRect(
    const widget::Rect<int32_t, uint32_t>& rect,
    float radiusX,
    float radiusY,
    const widget::Color& color
) {
    if (!renderTarget_) return;

    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (!brush) return;

    D2D1_ROUNDED_RECT roundedRect;
    roundedRect.rect = toD2DRect(rect);
    roundedRect.radiusX = radiusX;
    roundedRect.radiusY = radiusY;

    renderTarget_->FillRoundedRectangle(roundedRect, brush);
    brush->Release();
}

void RendererD2D::drawTextEx(
    const std::wstring& text,
    const widget::Rect<int32_t, uint32_t>& rect,
    const widget::Color& color,
    const FontStyle& font,
    TextAlign halign,
    VerticalAlign valign
) {
    if (!renderTarget_ || !writeFactory_) return;

    // Create text format with specified font
    IDWriteTextFormat* textFormat = nullptr;
    HRESULT hr = writeFactory_->CreateTextFormat(
        font.family.c_str(),
        nullptr,
        font.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        font.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font.size,
        L"en-us",
        &textFormat
    );

    if (FAILED(hr)) return;

    // Set text alignment
    switch (halign) {
        case TextAlign::Left:
            textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            break;
        case TextAlign::Center:
            textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            break;
        case TextAlign::Right:
            textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
            break;
        case TextAlign::Justify:
            textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
            break;
    }

    // Set vertical alignment
    switch (valign) {
        case VerticalAlign::Top:
            textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
            break;
        case VerticalAlign::Middle:
            textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            break;
        case VerticalAlign::Bottom:
            textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
            break;
    }

    // Draw text
    ID2D1SolidColorBrush* brush = createSolidBrush(color);
    if (brush) {
        renderTarget_->DrawTextW(
            text.c_str(),
            static_cast<UINT32>(text.length()),
            textFormat,
            toD2DRect(rect),
            brush
        );
        brush->Release();
    }

    textFormat->Release();
}

ID2D1Bitmap* RendererD2D::loadBitmapFromFile(const std::wstring& path) {
    if (!renderTarget_ || !wicFactory_) return nullptr;

    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    ID2D1Bitmap* bitmap = nullptr;

    // Decode image file
    HRESULT hr = wicFactory_->CreateDecoderFromFilename(
        path.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );

    if (FAILED(hr)) goto cleanup_loadBitmap;

    // Get first frame
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) goto cleanup_loadBitmap;

    // Convert to 32bppPBGRA format (required by Direct2D)
    hr = wicFactory_->CreateFormatConverter(&converter);
    if (FAILED(hr)) goto cleanup_loadBitmap;

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeMedianCut
    );
    if (FAILED(hr)) goto cleanup_loadBitmap;

    // Create D2D bitmap from WIC bitmap
    hr = renderTarget_->CreateBitmapFromWicBitmap(
        converter,
        nullptr,
        &bitmap
    );

cleanup_loadBitmap:
    if (converter) converter->Release();
    if (frame) frame->Release();
    if (decoder) decoder->Release();

    return bitmap;  // Returns nullptr on failure
}

void RendererD2D::save() {
    if (!renderTarget_) return;
    
    D2D1_MATRIX_3X2_F currentTransform;
    renderTarget_->GetTransform(&currentTransform);
    transformStack_.push(currentTransform);
}

void RendererD2D::drawBitmap(
    void* bitmapPtr, 
    const widget::Rect<int32_t, uint32_t>& destRect,
    float opacity
) {
    if (!renderTarget_ || !bitmapPtr) return;

    auto* bitmap = static_cast<ID2D1Bitmap*>(bitmapPtr);

    D2D1_RECT_F dest = toD2DRect(destRect);

    renderTarget_->DrawBitmap(
        bitmap,
        dest,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        nullptr  // Source rect (nullptr = entire bitmap)
    );
}

void RendererD2D::restore() {
    if (!renderTarget_ || transformStack_.empty()) return;
    
    D2D1_MATRIX_3X2_F savedTransform = transformStack_.top();
    transformStack_.pop();
    renderTarget_->SetTransform(savedTransform);
}

void RendererD2D::setOpacity(float opacity) {
    // TODO: Implement opacity layer
    (void)opacity;
}

void RendererD2D::setTransform(float m11, float m12, float m21, float m22,
                              float dx, float dy) {
    if (!renderTarget_) return;

    D2D1_MATRIX_3X2_F matrix;
    matrix._11 = m11;
    matrix._12 = m12;
    matrix._21 = m21;
    matrix._22 = m22;
    matrix._31 = dx;
    matrix._32 = dy;

    renderTarget_->SetTransform(matrix);
}

void RendererD2D::translate(float dx, float dy) {
    if (!renderTarget_) return;
    
    D2D1_MATRIX_3X2_F current;
    renderTarget_->GetTransform(&current);
    
    D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(dx, dy);
    D2D1_MATRIX_3X2_F combined = current * translation;
    
    renderTarget_->SetTransform(combined);
}

// ============================================================================
// TEXT MEASUREMENT IMPLEMENTATION (NEW!)
// ============================================================================

float RendererD2D::measureTextWidth(
    const std::wstring& text, 
    size_t length,
    const FontStyle& font
) const {
    if (!writeFactory_ || text.empty() || length == 0) return 0.0f;
    
    // Clamp length
    length = std::min(length, text.length());
    
    // Create text format for measurement
    IDWriteTextFormat* textFormat = nullptr;
    HRESULT hr = writeFactory_->CreateTextFormat(
        font.family.c_str(),
        nullptr,
        font.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        font.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font.size,
        L"en-us",
        &textFormat
    );
    
    if (FAILED(hr)) return 0.0f;
    
    // Create text layout for precise measurement
    IDWriteTextLayout* textLayout = nullptr;
    hr = writeFactory_->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(length),  // Only measure up to position
        textFormat,
        10000.0f,  // Max width (large enough)
        100.0f,    // Max height
        &textLayout
    );
    
    textFormat->Release();
    
    if (FAILED(hr)) return 0.0f;
    
    // Get text metrics
    DWRITE_TEXT_METRICS metrics;
    hr = textLayout->GetMetrics(&metrics);
    
    float width = 0.0f;
    if (SUCCEEDED(hr)) {
        width = metrics.width;
    }
    
    textLayout->Release();
    
    return width;
}

size_t RendererD2D::getCharPositionFromX(
    const std::wstring& text,
    float x,
    const FontStyle& font
) const {
    if (!writeFactory_ || text.empty() || x <= 0.0f) return 0;
    
    // Create text format
    IDWriteTextFormat* textFormat = nullptr;
    HRESULT hr = writeFactory_->CreateTextFormat(
        font.family.c_str(),
        nullptr,
        font.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
        font.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font.size,
        L"en-us",
        &textFormat
    );
    
    if (FAILED(hr)) return 0;
    
    // Create text layout
    IDWriteTextLayout* textLayout = nullptr;
    hr = writeFactory_->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(text.length()),
        textFormat,
        10000.0f,
        100.0f,
        &textLayout
    );
    
    textFormat->Release();
    
    if (FAILED(hr)) return 0;
    
    // Hit test to find character position
    BOOL isTrailingHit = FALSE;
    BOOL isInside = FALSE;
    DWRITE_HIT_TEST_METRICS hitMetrics;
    
    hr = textLayout->HitTestPoint(
        x,
        0.0f,
        &isTrailingHit,
        &isInside,
        &hitMetrics
    );
    
    size_t position = 0;
    
    if (SUCCEEDED(hr)) {
        position = static_cast<size_t>(hitMetrics.textPosition);
        
        // If hit trailing edge of character, move to next position
        if (isTrailingHit && position < text.length()) {
            position++;
        }
    }
    
    textLayout->Release();
    
    return std::min(position, text.length());
}

// ============================================================================
// RESOURCE MANAGEMENT
// ============================================================================

void RendererD2D::resize(uint32_t width, uint32_t height) {
    if (!renderTarget_) return;

    renderTarget_->Resize(D2D1::SizeU(width, height));
}

bool RendererD2D::recreateDeviceResources() {
    cleanupDeviceResources();

    if (!factory_ || !hwnd_) return false;

    RECT rc;
    GetClientRect(hwnd_, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(
        rc.right - rc.left,
        rc.bottom - rc.top
    );

    HRESULT hr = factory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd_, size),
        &renderTarget_
    );

    return SUCCEEDED(hr);
}

// ============================================================================
// HELPER METHODS
// ============================================================================

ID2D1SolidColorBrush* RendererD2D::createSolidBrush(const widget::Color& color) {
    if (!renderTarget_) return nullptr;

    ID2D1SolidColorBrush* brush = nullptr;
    renderTarget_->CreateSolidColorBrush(
        toD2DColor(color),
        &brush
    );

    return brush;
}

D2D1_COLOR_F RendererD2D::toD2DColor(const widget::Color& color) const noexcept {
    return D2D1::ColorF(
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f
    );
}

D2D1_RECT_F RendererD2D::toD2DRect(
    const widget::Rect<int32_t, uint32_t>& rect
) const noexcept {
    return D2D1::RectF(
        static_cast<float>(rect.x),
        static_cast<float>(rect.y),
        static_cast<float>(rect.getRight()),
        static_cast<float>(rect.getBottom())
    );
}

void RendererD2D::cleanup() noexcept {
    cleanupDeviceResources();

    while (!clipStack_.empty()) clipStack_.pop();
    while (!transformStack_.empty()) transformStack_.pop();

    if (defaultTextFormat_) {
        defaultTextFormat_->Release();
        defaultTextFormat_ = nullptr;
    }

    if (writeFactory_) {
        writeFactory_->Release();
        writeFactory_ = nullptr;
    }

    if (wicFactory_) {
        wicFactory_->Release();
        wicFactory_ = nullptr;
    }

    if (factory_) {
        factory_->Release();
        factory_ = nullptr;
    }
}

void RendererD2D::cleanupDeviceResources() noexcept {
    if (renderTarget_) {
        renderTarget_->Release();
        renderTarget_ = nullptr;
    }
}

} // namespace frqs::render