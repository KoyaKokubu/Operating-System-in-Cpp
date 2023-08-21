/*
* provide functionality to overlay windows
*/

#pragma once

#include <memory>
#include <map>
#include <vector>

#include "graphics.hpp"
#include "window.hpp"
#include "message.hpp"

/** @brief Layer indicates one layer 
*
*   In the future, this will have multiple windows.
*   At present, this can have only one though...
*/
class Layer {
    public:
        /** @brief generate a layer with given ID */
        Layer(unsigned int id = 0);
        /** @brief return ID of this instance */
        unsigned int ID() const;

        /** @brief former window will vanish from this layer */
        Layer& SetWindow(const std::shared_ptr<Window>& window);
        /** @brief return window which has been set */
        std::shared_ptr<Window> GetWindow() const;
        /** @brief get the origin point of this layer */
        Vector2D<int> GetPosition() const;
        /** @brief If true, layer can be dragged */
        Layer& SetDraggable(bool draggable);
        /** @brief return true if this layer can be dragged */
        bool IsDraggable() const;

        /** @brief move layer to the given absolute coordinates without redraw. */
        Layer& Move(Vector2D<int> pos);
        /** @brief move layer to the given relative coordinates without redraw. */
        Layer& MoveRelative(Vector2D<int> pos_diff);

        /** @brief draw content of window into the given screen */
        void DrawTo(FrameBuffer& screen, const Rectangle<int>& area) const;

    private:
        unsigned int id_;
        Vector2D<int> pos_{};
        std::shared_ptr<Window> window_{};
        bool draggable_{false};
};

/** @brief LayerManager manages multiple layers. */
class LayerManager {
    public:
        /** @brief set object to draw via Draw method and so on */
        void SetWriter(FrameBuffer* screen);
        /** @brief generate a new layer and return its reference 
        *
        *   the new instance will be hold in a container in LayerManager 
        */
        Layer& NewLayer();
				/** @brief remove the given layer */
				void RemoveLayer(unsigned int id);

        unsigned int GetLatestID() const;

        /** @brief draw layer which is in a state to be displayed */
        void Draw(const Rectangle<int>& area) const;
        /** @brief redraw display area of the window of the given layer */
        void Draw(unsigned int id) const;
        /** @brief redraw the area in a state to be displayed in the given layer  */
        void Draw(unsigned int id, Rectangle<int> area) const;

        /** @brief move layer to the given absolute coordinates, and redraw. */
        void Move(unsigned int id, Vector2D<int> new_pos);
        /** @brief move layer to the given relative coordinates, and redraw. */
        void MoveRelative(unsigned int id, Vector2D<int> pos_diff);

        /** @brief move layer to th given height. 
        *
        *   If the given height < 0, layer will not be displayed.
        *   If the given height >= 0, layer will set to the height.
        *   if the given height > num of layer, layer will be a front layer.
        */
        void UpDown(unsigned int id, int new_height);
        /** @brief set this layer undisplayed */
        void Hide(unsigned int id);

        /** @brief find the highest layer which has a window obtaining the given point. */
        Layer* FindLayerByPosition(Vector2D<int> pos, unsigned int exclude_id) const;
        /** @brief return the layer which owns the given ID */
        Layer* FindLayer(unsigned int id);
        /** @brief return the current height of the given layer */
        int GetHeight(unsigned int id);

    private:
        FrameBuffer* screen_{nullptr};
        mutable FrameBuffer back_buffer_{};
        std::vector<std::unique_ptr<Layer>> layers_{};
        std::vector<Layer*> layer_stack_{};
        unsigned int latest_id_{0};
};

extern LayerManager* layer_manager;

class ActiveLayer {
    public:
        ActiveLayer(LayerManager& manager);
        unsigned int GetMouseLayer();
        void SetMouseLayer(unsigned int mouse_layer);
        void Activate(unsigned int layer_id);
        unsigned int GetActive() const { return active_layer_; }
    
    private:
        LayerManager& manager_;
        unsigned int active_layer_{0};
        unsigned int mouse_layer_{0};
};

extern ActiveLayer* active_layer;
extern std::map<unsigned int, uint64_t>* layer_task_map;

void InitializeLayer();
void ProcessLayerMessage(const Message& msg);

constexpr Message MakeLayerMessage(
        uint64_t task_id, unsigned int layer_id,
        LayerOperation op, const Rectangle<int>& area) {
    Message msg{Message::kLayer, task_id};
    msg.arg.layer.layer_id = layer_id;
    msg.arg.layer.op = op;
    msg.arg.layer.x = area.pos.x;
    msg.arg.layer.y = area.pos.y;
    msg.arg.layer.w = area.size.x;
    msg.arg.layer.h = area.size.y;
    return msg;
}

Error CloseLayer(unsigned int layer_id);