#include <stdio.h>
#include <png/lodepng.h>
#include <gfx/bitmap.h>

Bitmap::Bitmap(const nk::String &path)
    : ownsBuffer(true), format(PixelFormat::Rgba32), bpp(4)
{
    FILE *fd = fopen(path.CStr(), "r");
    if (!fd)
        return; // TODO: Error

    fseek(fd, 0, SEEK_END);
    size_t len = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    uint8_t *data = new uint8_t[len];
    fread(data, 1, len, fd);
    fclose(fd);

    unsigned error = lodepng_decode_memory(&this->data, &width, &height, data, len, LodePNGColorType::LCT_RGBA, 8);
    stride = width * bpp;
}

Bitmap::Bitmap(unsigned int width, unsigned int height, unsigned int stride, uint8_t *data, PixelFormat format)
    : width(width), height(height), stride(stride), data(data), format(format), ownsBuffer(false)
{
    bpp = format == PixelFormat::Bgr32 ? 4 : 3;
}

Bitmap::~Bitmap()
{
    if (ownsBuffer)
        delete[] data;
}

void Bitmap::SetPixel(unsigned int x, unsigned int y, Color pixel)
{
    size_t baseIdx = GetIndex(x, y);
    if (format == PixelFormat::Rgb24)
    {
        data[baseIdx + 0] = pixel.r;
        data[baseIdx + 1] = pixel.g;
        data[baseIdx + 2] = pixel.b;
    }
    else if (format == PixelFormat::Bgr32)
    {
        data[baseIdx + 2] = pixel.r;
        data[baseIdx + 1] = pixel.g;
        data[baseIdx + 0] = pixel.b;
    }
    else if (format == PixelFormat::Rgba32)
    {
        data[baseIdx + 0] = pixel.r;
        data[baseIdx + 1] = pixel.g;
        data[baseIdx + 2] = pixel.b;
        data[baseIdx + 3] = pixel.a;
    }
}

Color Bitmap::GetPixel(unsigned int x, unsigned int y) const
{
    size_t baseIdx = GetIndex(x, y);
    if (format == PixelFormat::Rgb24)
    {
        return {data[baseIdx + 0], data[baseIdx + 1], data[baseIdx + 2], 1};
    }
    else if (format == PixelFormat::Bgr32)
    {
        return {data[baseIdx + 2], data[baseIdx + 1], data[baseIdx + 0], 1};
    }
    else if (format == PixelFormat::Rgba32)
    {
        return {data[baseIdx + 0], data[baseIdx + 1], data[baseIdx + 2], data[baseIdx + 3]};
    }
}

inline size_t Bitmap::GetIndex(unsigned int x, unsigned int y) const
{
    return y * stride + x * bpp;
}

void Bitmap::Blit(const Bitmap &other, const Rectangle &dstRect)
{
    for (size_t x = 0; x < dstRect.size.width; x++)
    {
        for (size_t y = 0; y < dstRect.size.height; y++)
        {
            auto px = other.GetPixel(x, y);
            SetPixel(x + dstRect.position.x, y + dstRect.position.y, px);
        }
    }
}

void Bitmap::DrawBitmap(const Bitmap &other, const Rectangle &dstRect)
{
    for (size_t x = 0; x < dstRect.size.width; x++)
    {
        for (size_t y = 0; y < dstRect.size.height; y++)
        {
            auto srcPixel = GetPixel(x + dstRect.position.x, y + dstRect.position.y);
            auto dstPx = other.GetPixel(x, y);
            auto blended = Blend(srcPixel, dstPx);
            SetPixel(x + dstRect.position.x, y + dstRect.position.y, blended);
        }
    }
}

Color Bitmap::Blend(Color b, Color a)
{
    float alpha = a.a / 255.0f;
    float alphaInv = 1.0f - alpha;
    return {
        static_cast<uint8_t>(alpha * (float)a.r + alphaInv * (float)b.r),
        static_cast<uint8_t>(alpha * (float)a.g + alphaInv * (float)b.g),
        static_cast<uint8_t>(alpha * (float)a.b + alphaInv * (float)b.b),
        255
    };
}