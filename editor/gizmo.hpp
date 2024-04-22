#include "renderer/sprite.hpp"

namespace looper {

enum class GizmoState
{
   rotate,
   translate,
   scale
};

enum class GizmoPart
{
   center,
   vertical,
   hotizontal,
   none
};


class Gizmo
{
 public:
   void
   Initialize();
   void
   Show();
   void
   Hide();
   void
   Render();
   void
   Update(const glm::vec2& centeredPos, float rotation);
   void
   Move(const glm::vec2& moveBy);
   void
   Zoom(int32_t zoomVal);

   void
   CheckHovered(const glm::vec3& cameraPos, const glm::vec2& globalPosition);

   void
   SwitchToScale();
   void
   SwitchToRotate();
   void
   SwitchToTranslate();

   glm::vec2
   Position() const;

 private:
   void
   AdjustSize();

 public:
   GizmoState currentState_ = GizmoState::translate;
   GizmoPart selectedPart_ = GizmoPart::none;
   bool mouseOnGizmo_ = false;

 private:
   renderer::Sprite gizmoCenter_ = {};
   renderer::Sprite gizmoUp_ = {};
   renderer::Sprite gizmoSide_ = {};

   int32_t zoomLevel_ = 0;
   float currentRotation_ = 0.0f;
   // <default, rotate>
   std::pair< glm::vec2, glm::vec2 > centerInitialSize_ = {};
   glm::vec2 upInitialSize_ = {};
   glm::vec2 sideInitialSize_ = {};

   // <default, rotate>
   std::pair< glm::vec2, glm::vec2 > centerCurrentSize_ = {};
   glm::vec2 upCurrentSize_ = {};
   glm::vec2 sideCurrentSize_ = {};
};
} // namespace looper
