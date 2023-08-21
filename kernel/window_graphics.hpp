#pragma once

#include "graphics.hpp"
#include "window.hpp"

void DrawRectangle(Window& window, const Vector2D<int>& pos,
                    const Vector2D<int>& size, const PixelColor& c);

void FillRectangle(Window& window, const Vector2D<int>& pos,
                    const Vector2D<int>& size, const PixelColor& c);

void DrawDesktop(Window& window);
