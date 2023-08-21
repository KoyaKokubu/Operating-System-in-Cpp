/**
* @file main.cpp
* program for kernel
*/

#include <cstdint>
#include <cstddef>
#include <cstdio>

#include <memory>
#include <deque>
#include <limits>
#include <numeric>
#include <vector>

#include "frame_buffer_config.hpp"
#include "memory_map.hpp"
#include "graphics.hpp"
#include "mouse.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"
#include "logger.hpp"
#include "usb/xhci/xhci.hpp"
#include "interrupt.hpp"
#include "asmfunc.h"
#include "segment.hpp"
#include "paging.hpp"
#include "memory_manager.hpp"
#include "window.hpp"
#include "layer.hpp"
#include "window_graphics.hpp"
#include "message.hpp"
#include "timer.hpp"
#include "acpi.hpp"
#include "keyboard.hpp"
#include "task.hpp"
#include "terminal.hpp"
#include "fat.hpp"
#include "syscall.hpp"

int printk(const char* format, ...) {
    va_list ap;
    int result;
    char s[1024];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    console->PutString(s);
    return result;
}

std::shared_ptr<ToplevelWindow> main_window;
unsigned int main_window_layer_id;
void InitializeMainWindow() {
    main_window = std::make_shared<ToplevelWindow>(
        160, 52, screen_config.pixel_format, "Hello Window"
    );

    main_window_layer_id = layer_manager->NewLayer()
        .SetWindow(main_window)
        .SetDraggable(true)
        .Move({300, 100})
        .ID();

    layer_manager->UpDown(main_window_layer_id, std::numeric_limits<int>::max());
}

std::shared_ptr<ToplevelWindow> text_window;
unsigned int text_window_layer_id;
void InitializeTextWindow() {
    const int win_w = 160;
    const int win_h = 52;

    text_window = std::make_shared<ToplevelWindow>(
        win_w, win_h, screen_config.pixel_format, "Text Box Test"
    );
    #ifdef WINDOW_WRITER
        DrawTextbox(*text_window->InnerWriter(), {0, 0}, text_window->InnerSize());
    #else
        DrawTextbox(*text_window->InnerWriter(), {0, 0}, text_window->InnerSize());
        // DrawWindow(*text_window, "Text Box Test");
        // DrawTextbox(*text_window, {4, 24}, {win_w-8, win_h-24-4});
    #endif

    text_window_layer_id = layer_manager->NewLayer()
        .SetWindow(text_window)
        .SetDraggable(true)
        .Move({500, 100})
        .ID();

    layer_manager->UpDown(text_window_layer_id, std::numeric_limits<int>::max());
}

int text_window_index;

void DrawTextCursor(bool visible) {
    const auto color = visible ? ToColor(windowFGColor) : ToColor(windowBGColor);
    const auto pos = Vector2D<int>{4 + 8*text_window_index, 5};
    #ifdef WINDOW_WRITER
        FillRectangle(*text_window->InnerWriter(), pos, {1, 15}, color);
    #else
        FillRectangle(*text_window->InnerWriter(), pos, {1, 15}, color);
        // FillRectangle(*text_window, pos, {1, 15}, color);
    #endif
}

void InputTextWindow(char c) {
    if (c == 0) {
        return;
    }

    auto pos = []() { return Vector2D<int>{4 + 8*text_window_index, 6}; };

    const int max_chars = (text_window->InnerSize().x - 8) / 8 - 1;
    if (c == '\b' && text_window_index > 0) {
        DrawTextCursor(false);
        --text_window_index;
        #ifdef WINDOW_WRITER
            FillRectangle(*text_window->InnerWriter(), pos(), {8, 16}, ToColor(windowBGColor));
        #else
            FillRectangle(*text_window->InnerWriter(), pos(), {8, 16}, ToColor(windowBGColor));
            // FillRectangle(*text_window, pos(), {8, 16}, ToColor(windowBGColor));
        #endif
        DrawTextCursor(true);
    } else if (c >= ' ' && text_window_index < max_chars) {
        DrawTextCursor(false);
        #ifdef WINDOW_WRITER
            WriteAscii(*text_window->InnerWriter(), pos(), c, ToColor(windowFGColor));
        #else
            WriteAscii(*text_window->InnerWriter(), pos(), c, ToColor(windowFGColor));
            // WriteAscii(*text_window, pos(), c, ToColor(windowFGColor));
        #endif
        ++text_window_index;
        DrawTextCursor(true);
    }

    layer_manager->Draw(text_window_layer_id);
}

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void KernelMainNewStack(const FrameBufferConfig& frame_buffer_config_ref,
                                    const MemoryMap& memory_map_ref,
                                    const acpi::RSDP& acpi_table,
                                    void* volume_image) {
    MemoryMap memory_map{memory_map_ref};

    InitializeGraphics(frame_buffer_config_ref);
    InitializeConsole();

    printk("Welcome to MikanOS!\n");
    printk("Name: Koya\n");
    SetLogLevel(kWarn);

    InitializeSegmentation();
    InitializePaging();
    InitializeMemoryManager(memory_map);
		InitializeTSS();
    InitializeInterrupt();

    fat::Initialize(volume_image);
		InitializeFont();
    InitializePCI();

    InitializeLayer();
    InitializeMainWindow();
    InitializeTextWindow();
    layer_manager->Draw({{0, 0}, ScreenSize()});

    acpi::Initialize(acpi_table);
    InitializeLAPICTimer();

    const int kTextboxCursorTimer = 1;
    const int kTimer05Sec = static_cast<int>(kTimerFreq * 0.5);
    timer_manager->AddTimer(Timer{kTimer05Sec, kTextboxCursorTimer, 1});
    bool textbox_cursor_visible = false;

		InitializeSyscall();

    InitializeTask();
    Task& main_task = task_manager->CurrentTask();

    usb::xhci::Initialize();
    InitializeKeyboard();
    InitializeMouse();

		app_loads = new std::map<fat::DirectoryEntry*, AppLoadInfo>;
    task_manager->NewTask()
        .InitContext(TaskTerminal, 0)
        .Wakeup();

    char str[128];

    while (true) {
        __asm__("cli");
        const auto tick = timer_manager->CurrentTick();
        __asm__("sti");

        sprintf(str, "%010lu", tick);
        #ifdef WINDOW_WRITER
            FillRectangle(*main_window->InnerWriter(), {20, 4}, {8*10, 16}, {0x32, 0x32, 0x32});
            WriteString(*main_window->InnerWriter(), {20, 4}, str, ToColor(windowFGColor));
        #else
            FillRectangle(*main_window->InnerWriter(), {20, 4}, {8*10, 16}, {0x32, 0x32, 0x32});
            WriteString(*main_window->InnerWriter(), {20, 4}, str, ToColor(windowFGColor));
            // FillRectangle(*main_window, {24, 28}, {8 * 10, 16}, {0x32, 0x32, 0x32});
            // WriteString(*main_window, {24, 28}, str, ToColor(windowFGColor));
        #endif
        layer_manager->Draw(main_window_layer_id);

        __asm__("cli");
        auto msg = main_task.ReceiveMessage();
        if (!msg) {
            main_task.Sleep();
            __asm__("sti");
            continue;
        }

        __asm__("sti");

        switch (msg->type) {
            case Message::kInterruptXHCI:
                usb::xhci::ProcessEvents();
                break;
            case Message::kTimerTimeout:
                if (msg->arg.timer.value == kTextboxCursorTimer) {
                    __asm__("cli");
                    timer_manager->AddTimer(
                        Timer{msg->arg.timer.timeout + kTimer05Sec, kTextboxCursorTimer, 1}
                    );
                    __asm__("sti");
                    textbox_cursor_visible = !textbox_cursor_visible;
                    DrawTextCursor(textbox_cursor_visible);
                    layer_manager->Draw(text_window_layer_id);
                }
                break;
            case Message::kKeyPush:
                {
										auto act = active_layer->GetActive();
                    const bool alt = (msg->arg.keyboard.modifier & (kLAltBitMask | kRAltBitMask)) != 0;
                    if (alt && msg->arg.keyboard.ascii == 'a') {
												if (msg->arg.keyboard.press) {
														unsigned int act_id = act + 1;
														while (act_id == 1 || act_id == 2) { act_id += 1; }
														if (act_id == active_layer->GetMouseLayer()) { act_id += 1; }
														while (act_id != 0 && !layer_manager->FindLayer(act_id)) { 
																if (act_id > layer_manager->GetLatestID()) { act_id = 0; }
																else { act_id += 1; }
														}
														while (act_id == 1 || act_id == 2) { act_id += 1; }
														active_layer->Activate(act_id);
												}
                    } else if (act == text_window_layer_id) {
												if (msg->arg.keyboard.press) {
														InputTextWindow(msg->arg.keyboard.ascii);
												}
                    } else if (msg->arg.keyboard.press &&
																msg->arg.keyboard.keycode == 59 /* F2 */) {
												task_manager->NewTask()
													.InitContext(TaskTerminal, 0)
													.Wakeup();
										} else {
                        __asm__("cli");
                        auto task_it = layer_task_map->find(act);
                        __asm__("sti");
                        if (task_it != layer_task_map->end()) {
                            __asm__("cli");
                            task_manager->SendMessage(task_it->second, *msg);
                            __asm__("sti");
                        } else {
														if (msg->arg.keyboard.press) {
																printk("key push not handled: keycode %02x, ascii %02x\n",
																		msg->arg.keyboard.keycode,
																		msg->arg.keyboard.ascii);
														}
                        }
                    }
                }
                break;
            case Message::kLayer:
                ProcessLayerMessage(*msg);
                __asm__("cli");
                task_manager->SendMessage(msg->src_task, Message{Message::kLayerFinish});
                __asm__("sti");
                break;
            default:
                Log(kError, "Unknown message type: %d\n", msg->type);
        }
    }
}

extern "C" void __cxa_pure_virtual() {
    while (1) __asm__("hlt");
}