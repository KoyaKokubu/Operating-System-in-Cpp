#include "graphics.hpp"
#include "window_graphics.hpp"

#ifndef WINDOW_WRITER
    void DrawRectangle(Window& window, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c) {
        for (int dx = 0; dx < size.x; ++dx) {
            window.Write(pos + Vector2D<int>{dx, 0}, c);
            window.Write(pos + Vector2D<int>{dx, size.y - 1}, c);        
        }
        for (int dy = 1; dy < size.y - 1; ++dy) {
            window.Write(pos + Vector2D<int>{0, dy}, c);
            window.Write(pos + Vector2D<int>{size.x - 1, dy}, c);
        }
    }

    void FillRectangle(Window& window, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c) {
        for (int dy = 0; dy < size.y; ++dy) {
            for (int dx = 0; dx < size.x; ++dx) {
                window.Write(pos + Vector2D<int>{dx, dy}, c);
            }
        }
    }

    void DrawDesktop(Window& window) {
        const auto width = window.Width();
        const auto height = window.Height();
        FillRectangle(window,
                        {0, 0},
                        {width, height-50},
                        kDesktopBGColor);
        FillRectangle(window,
                        {0, height-50},
                        {width, 50},
                        kDesktopBtmRhtColor);
        FillRectangle(window,
                        {0, height-50},
                        {width/5, 50},
                        kDesktopBtmLftColor);
        DrawRectangle(window,
                        {10, height-40},
                        {30, 30},
                        {160, 160, 160});
    }
#endif