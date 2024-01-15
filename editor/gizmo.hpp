#include "renderer/sprite.hpp"

namespace looper {

struct Gizmo
{
   void
   Initialize();
   void
   Show();
   void
   Hide();
   void
   Render();
   void
   NewObjectSelected(const glm::vec2& centeredPos);

   void
   CheckHovered(const glm::vec3& cameraPos, const glm::vec2& globalPosition);
   void
   SwitchToScale();
   void
   SwitchToRotate();
   void
   SwitchToTranslate();

   renderer::Sprite gizmoCenter_ = {};
   renderer::Sprite gizmoUp_ = {};
   renderer::Sprite gizmoSide_ = {};
};
} // namespace looper
