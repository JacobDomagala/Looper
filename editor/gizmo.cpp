#include "gizmo.hpp"

namespace looper {

void
Gizmo::Initialize()
{
   gizmoCenter_.SetSpriteTextured(glm::vec3{}, {16, 16}, "rounded_square.png");
   gizmoCenter_.SetColor({0.3f, 0.3f, 0.3f, 1.0f});

   gizmoUp_.SetSpriteTextured(glm::vec3{}, {96, 32}, "arrow.png");
   gizmoUp_.Rotate(90.0f, renderer::Sprite::RotationType::DEGREES);
   gizmoUp_.SetColor({0.0f, 1.0f, 0.0f, 1.0f});

   gizmoSide_.SetSpriteTextured(glm::vec3{}, {96, 32}, "arrow.png");
   gizmoSide_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});
}

void
Gizmo::Show()
{
   gizmoCenter_.Show();
   gizmoSide_.Show();
   gizmoUp_.Show();
}
void
Gizmo::Hide()
{
   gizmoCenter_.Hide();
   gizmoSide_.Hide();
   gizmoUp_.Hide();
}
void
Gizmo::Render()
{
   gizmoCenter_.Render();
   gizmoSide_.Render();
   gizmoUp_.Render();
}
void
Gizmo::NewObjectSelected(const glm::vec2& centeredPos)
{
   gizmoCenter_.SetInitialPosition(glm::vec3{centeredPos, 0.0f});

   gizmoUp_.SetInitialPosition(
      gizmoCenter_.GetPosition()
      + glm::vec3{0.0f, gizmoCenter_.GetSize().y / 2.0f + gizmoUp_.GetSize().x / 1.85f, 0.0f});


   gizmoSide_.SetInitialPosition(
      gizmoCenter_.GetPosition()
      + glm::vec3{glm::vec2(gizmoCenter_.GetSize().x / 2.0f + gizmoSide_.GetSize().x / 1.85f, 0.0f),
                  0.0f});
}

void
Gizmo::CheckHovered(const glm::vec3& cameraPos, const glm::vec2& globalPosition)
{
   bool gizmoTouched = false;
   auto checkGizmo = [&gizmoTouched, cameraPos, globalPosition](auto& gizmo) {
      if (not gizmoTouched and gizmo.CheckIfCollidedScreenPosion(cameraPos, globalPosition))
      {
         gizmo.SetSize(gizmo.initialSize_ * 1.1f);
         gizmoTouched = true;
      }
      else
      {
         gizmo.SetSize(gizmo.initialSize_);
      }
   };

   checkGizmo(gizmoCenter_);
   checkGizmo(gizmoSide_);
   checkGizmo(gizmoUp_);
}
void
Gizmo::SwitchToScale()
{
   gizmoSide_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                           renderer::TextureLibrary::GetTexture("scale_slider.png")->GetID());
   gizmoUp_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                         renderer::TextureLibrary::GetTexture("scale_slider.png")->GetID());
}
void
Gizmo::SwitchToRotate()
{
}
void
Gizmo::SwitchToTranslate()
{
   gizmoSide_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                           renderer::TextureLibrary::GetTexture("arrow.png")->GetID());
   gizmoUp_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                         renderer::TextureLibrary::GetTexture("arrow.png")->GetID());
}

} // namespace looper