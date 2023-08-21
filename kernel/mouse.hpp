#pragma once

#include <memory>

#include "graphics.hpp"
#include "window.hpp"

const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
const PixelColor kMouseTransparentColor{0, 0, 1};

#ifdef WINDOW_WRITER
    void DrawMouseCursor(PixelWriter* pixel_writer, Vector2D<int> position);
#else
    void DrawMouseCursor(std::shared_ptr<Window>& window, Vector2D<int> position);
#endif

class Mouse {
    public:
        Mouse(unsigned int layer_id);
        void OnInterrupt(uint8_t buttons, int8_t displacement_x, int8_t displacement_y);

        unsigned int LayerID() const { return layer_id_; }
        void SetPosition(Vector2D<int> position);
        Vector2D<int> Position() const { return position_; }

    private:
        unsigned int layer_id_;
        Vector2D<int> position_{};

        unsigned int drag_layer_id_{0};
        uint8_t previous_buttons_{0};
};

void InitializeMouse();