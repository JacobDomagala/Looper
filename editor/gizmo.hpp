#include "renderer/sprite.hpp"

namespace looper {

enum class GizmoState
{
	rotate,
	translate,
	scale
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
   NewObjectSelected(const glm::vec2& centeredPos, float rotation);

   void
   CheckHovered(const glm::vec3& cameraPos, const glm::vec2& globalPosition);
   void
   SwitchToScale();
   void
   SwitchToRotate();
   void
   SwitchToTranslate();

   private:
   renderer::Sprite gizmoCenter_ = {};
   renderer::Sprite gizmoUp_ = {};
   renderer::Sprite gizmoSide_ = {};

   float currentRotation_ = 0.0f;
   // <default, rotate>
   std::pair< glm::vec2, glm::vec2 > centerInitialSize_ = {};
   glm::vec2 upInitialSize_ = {};
   glm::vec2 sideInitialSize_ = {};

   GizmoState currentState_ = GizmoState::translate;
};
} // namespace looper
