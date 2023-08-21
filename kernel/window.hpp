/*
* provide Window class indicating display area
*/

#pragma once

#include <vector>
#include <optional>
#include <string>
#include "graphics.hpp"
#include "frame_buffer.hpp"

#define WINDOW_WRITER

enum class WindowRegion {
		kTitleBar,
		kCloseButton,
		kBorder,
		kOther,
};

/** @brief Window class indicates graphic display area.
*
*   this includes not only windows with title and menu, but also area of mouse cursor.
*/
#ifdef WINDOW_WRITER
    class Window {
        public:
            /** @brief WindowWriter provides PixelWriter related to Window*/
            class WindowWriter : public PixelWriter {
                public:
                    WindowWriter(Window& window) : window_{window} {}
                    /** @brief set the given color to the given point */
                    virtual void Write(Vector2D<int> pos, const PixelColor& c) override {
                        window_.Write(pos, c);
                    }
                    /** @brief returns width of related Window in a scale of pixel */
                    virtual int Width() const override {return window_.Width(); }
                    /** @brief returns height of related Window in a scale of pixel */
                    virtual int Height() const override {return window_.Height(); }
                
                private:
                    Window& window_;
            };

            /** @brief make plane area to draw which has the given pixels */
            Window(int width, int height, PixelFormat shadow_format);
            virtual ~Window() = default;
            Window(const Window& rhs) = delete;
            Window& operator=(const Window& rhs) = delete;

            /** @brief draw area of this window into the given FrameBuffer 
            *
            *   @param dst : object to draw
            *   @param pos : position to draw based on the top left of dst
            *   @param area : area to draw based on the top left of dst
            */
            void DrawTo(FrameBuffer& dst, Vector2D<int> pos, const Rectangle<int>& area);
            /** @brief set transparent color */
            void SetTransparentColor(std::optional<PixelColor> c);
            
            /** @brief return pixel of the given point */
            const PixelColor& At(Vector2D<int> pos) const;
            /** @brief write pixels into the given point */
            void Write(Vector2D<int> pos, PixelColor c);
            
            /** @brief get WindowWriter related to this instance */
            WindowWriter* Writer();
            /** @brief return width of the plane area to draw in a scale of pixel */
            int Width() const;
            /** @brief return height of the plane area to draw in a scale of pixel */
            int Height() const;
            /** @brief return size of the plane area to draw in a scale of pixel */
            Vector2D<int> Size() const;

            /** @brief move rectangle in the plane area of this window.
            *
            *   @param src_pos : origin point of the source rectangle
            *   @param src_size : size of the source rectangle
            *   @param dst_pos : origin point of the destination point
            */
            void Move(Vector2D<int> dst_pos, const Rectangle<int>& src);

            virtual void Activate() {}
            virtual void Deactivate() {}
						virtual WindowRegion GetWindowRegion(Vector2D<int> pos);
        
        private:
            int width_, height_;
            std::vector<std::vector<PixelColor>> data_{};
            WindowWriter writer_{*this};
            std::optional<PixelColor> transparent_color_{std::nullopt};

            FrameBuffer shadow_buffer_{};
    };

    void DrawWindow(PixelWriter& writer, const char* title);
    void DrawWindowTitle(PixelWriter& writer, const char* title, bool active);

#else

    class Window : public PixelWriter {
        public:
            /** @brief make plane area to draw which has the given pixels */
            Window(int width, int height, PixelFormat shadow_format);
            virtual ~Window() = default;
            Window(const Window& rhs) = delete;
            Window& operator=(const Window& rhs) = delete;

            /** @brief set the given color to the given point */
            virtual void Write(Vector2D<int> pos, const PixelColor& c) override {
                data_[pos.y][pos.x] = c;
                shadow_buffer_.Writer().Write(pos, c);
            }

            /** @brief draw area of this window into the given FrameBuffer 
            *
            *   @param dst : object to draw
            *   @param pos : position to draw based on the top left of dst
            *   @param area : area to draw based on the top left of dst
            */
            void DrawTo(FrameBuffer& dst, Vector2D<int> pos, const Rectangle<int>& area);
            void SetTransparentColor(std::optional<PixelColor> c);

            /** @brief return pixel of the given point */
            const PixelColor& At(Vector2D<int> pos) const;

            /** @brief return width of the plane area to draw in a scale of pixel */
            int Width() const override { return width_; };
            /** @brief return height of the plane area to draw in a scale of pixel */
            int Height() const override { return height_; };
            /** @brief return size of the plane area to draw in a scale of pixel */
            Vector2D<int> Size() const;

            /** @brief move rectangle in the plane area of this window.
            *
            *   @param src_pos : origin point of the source rectangle
            *   @param src_size : size of the source rectangle
            *   @param dst_pos : origin point of the destination point
            */
            void Move(Vector2D<int> dst_pos, const Rectangle<int>& src);

            virtual void Activate() {}
            virtual void Deactivate() {}
						virtual WindowRegion GetWindowRegion(Vector2D<int> pos);

        private:
            int width_, height_;
            std::vector<std::vector<PixelColor>> data_{};
            std::optional<PixelColor> transparent_color_{std::nullopt};
            
            FrameBuffer shadow_buffer_{};
    };

    void DrawWindow(Window& window, const char* title);
    void DrawWindowTitle(Window& window, const char* title, bool active);

#endif

class ToplevelWindow : public Window {
    public:
        static constexpr Vector2D<int> kTopLeftMargin{4, 24};
        static constexpr Vector2D<int> kBottomRightMargin{4, 4};
        static constexpr int kMarginX = kTopLeftMargin.x + kBottomRightMargin.x;
        static constexpr int kMarginY = kTopLeftMargin.y + kBottomRightMargin.y;

        class InnerAreaWriter : public PixelWriter {
            public:
                InnerAreaWriter(ToplevelWindow& window) : window_{window} {}
                virtual void Write(Vector2D<int> pos, const PixelColor& c) override {
                    window_.Write(pos + kTopLeftMargin, c);
                }
                virtual int Width() const override {
                    return window_.Width() - kTopLeftMargin.x - kBottomRightMargin.x;
                }
                virtual int Height() const override {
                    return window_.Height() - kTopLeftMargin.y - kBottomRightMargin.y;
                }
            
            private:
                ToplevelWindow& window_;
        };

        ToplevelWindow(int width, int height, PixelFormat shadow_format,
                        const std::string& title);
        
        virtual void Activate() override;
        virtual void Deactivate() override;
				virtual WindowRegion GetWindowRegion(Vector2D<int> pos) override;

        InnerAreaWriter* InnerWriter() { return &inner_writer_; }
        Vector2D<int> InnerSize() const;
        
    private:
        std::string title_;
        InnerAreaWriter inner_writer_{*this};
};

void DrawTextbox(PixelWriter& writer, Vector2D<int> pos, Vector2D<int> size);
void DrawTerminal(PixelWriter& writer, Vector2D<int> pos, Vector2D<int> size);