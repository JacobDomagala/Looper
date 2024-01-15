#include "gizmo.hpp"

namespace looper {

void
Gizmo::Initialize()
{
   gizmoCenter_.SetSpriteTextured(glm::vec3{}, {16, 16}, "centered_move.png");
   gizmoCenter_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

   gizmoUp_.SetSpriteTextured(glm::vec3{}, {96, 32}, "arrow.png");
   gizmoUp_.Rotate(90.0f, renderer::Sprite::RotationType::DEGREES);
   gizmoUp_.SetColor({0.0f, 1.0f, 0.0f, 1.0f});

   gizmoSide_.SetSpriteTextured(glm::vec3{}, {96, 32}, "arrow.png");
   gizmoSide_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});

   centerInitialSize_ = {{16, 16}, {96, 96}};
   upInitialSize_ = {96, 32};
   sideInitialSize_ = {96, 32};
}

void
Gizmo::Show()
{
   gizmoCenter_.Show();

   if (currentState_ != GizmoState::rotate)
   {
      gizmoSide_.Show();
      gizmoUp_.Show();
   }
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
Gizmo::NewObjectSelected(const glm::vec2& centeredPos, float rotation)
{
   currentRotation_ = rotation;

   if (currentState_ == GizmoState::rotate)
   {
      gizmoCenter_.Rotate(currentRotation_);
   }
   
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
   auto checkGizmo = [&gizmoTouched, cameraPos, globalPosition](auto& gizmo,
                                                                const glm::vec2& defaultSize) {
      if (not gizmoTouched and gizmo.CheckIfCollidedScreenPosion(cameraPos, globalPosition))
      {
         gizmo.SetSize(defaultSize * 1.1f);
         gizmoTouched = true;
      }
      else
      {
         gizmo.SetSize(defaultSize);
      }
   };

   checkGizmo(gizmoCenter_, currentState_ == GizmoState::rotate ? centerInitialSize_.second
                                                                : centerInitialSize_.first);
   checkGizmo(gizmoSide_, sideInitialSize_);
   checkGizmo(gizmoUp_, upInitialSize_);
}
void
Gizmo::SwitchToScale()
{
   currentState_ = GizmoState::scale;

   gizmoCenter_.SetSize(centerInitialSize_.first);
   gizmoCenter_.Rotate(0.0f);
   gizmoSide_.Show();
   gizmoUp_.Show();

   gizmoCenter_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                             renderer::TextureLibrary::GetTexture("rounded_square.png")->GetID());
   gizmoSide_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                           renderer::TextureLibrary::GetTexture("scale_slider.png")->GetID());
   gizmoUp_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                         renderer::TextureLibrary::GetTexture("scale_slider.png")->GetID());
}
void
Gizmo::SwitchToRotate()
{
   currentState_ = GizmoState::rotate;

   gizmoSide_.Hide();
   gizmoUp_.Hide();

   gizmoCenter_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                             renderer::TextureLibrary::GetTexture("rotate.png")->GetID());
   gizmoCenter_.SetSize(centerInitialSize_.second);
   gizmoCenter_.Rotate(currentRotation_);
}
void
Gizmo::SwitchToTranslate()
{
   currentState_ = GizmoState::translate;

   gizmoCenter_.SetSize(centerInitialSize_.first);
   gizmoCenter_.Rotate(0.0f);
   gizmoSide_.Show();
   gizmoUp_.Show();

   gizmoCenter_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                             renderer::TextureLibrary::GetTexture("centered_move.png")->GetID());
   gizmoSide_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                           renderer::TextureLibrary::GetTexture("arrow.png")->GetID());
   gizmoUp_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                         renderer::TextureLibrary::GetTexture("arrow.png")->GetID());
}

} // namespace looper