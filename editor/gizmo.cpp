#include "gizmo.hpp"
#include "types.hpp"

namespace looper {

void
Gizmo::Initialize()
{
   gizmoCenter_.SetSpriteTextured(glm::vec3{}, {16, 16}, "centered_move.png",
                                  renderer::SpriteType::alwaysOnTop);
   gizmoCenter_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

   gizmoUp_.SetSpriteTextured(glm::vec3{}, {96, 32}, "arrow.png",
                              renderer::SpriteType::alwaysOnTop);
   gizmoUp_.Rotate(90.0f, renderer::RotationType::degrees);
   gizmoUp_.SetColor({0.0f, 1.0f, 0.0f, 1.0f});

   gizmoSide_.SetSpriteTextured(glm::vec3{}, {96, 32}, "arrow.png",
                                renderer::SpriteType::alwaysOnTop);
   gizmoSide_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});

   centerInitialSize_ = {{16, 16}, {96, 96}};
   upInitialSize_ = {96, 32};
   sideInitialSize_ = {96, 32};

   Hide();
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
Gizmo::Update(const glm::vec2& centeredPos, float rotation)
{
   currentRotation_ = rotation;

   if (currentState_ == GizmoState::rotate)
   {
      gizmoCenter_.Rotate(currentRotation_);
   }

   gizmoCenter_.SetInitialPosition(glm::vec3{centeredPos, renderer::LAYER_0});

   gizmoUp_.SetInitialPosition(
      gizmoCenter_.GetPosition()
      + glm::vec3{0.0f, gizmoCenter_.GetSize().y / 2.0f + gizmoUp_.GetSize().x / 1.85f, 0.0f});


   gizmoSide_.SetInitialPosition(
      gizmoCenter_.GetPosition()
      + glm::vec3{glm::vec2(gizmoCenter_.GetSize().x / 2.0f + gizmoSide_.GetSize().x / 1.85f, 0.0f),
                  0.0f});
}

void
Gizmo::Move(const glm::vec2& moveBy)
{
   gizmoCenter_.Translate(moveBy);
   gizmoUp_.Translate(moveBy);
   gizmoSide_.Translate(moveBy);
}

void
Gizmo::CheckHovered(const glm::vec3& cameraPos, const glm::vec2& globalPosition)
{
   bool gizmoTouched = false;
   selectedPart_ = GizmoPart::none;

   auto checkGizmo = [this, &gizmoTouched, cameraPos,
                      globalPosition](auto& gizmo, const glm::vec2& defaultSize, GizmoPart part) {
      if (not gizmoTouched and gizmo.CheckIfCollidedScreenPosion(cameraPos, globalPosition))
      {
         gizmo.SetSize(defaultSize * 1.1f);
         gizmoTouched = true;
         selectedPart_ = part;
      }
      else
      {
         gizmo.SetSize(defaultSize);
      }
   };

   checkGizmo(gizmoCenter_,
              currentState_ == GizmoState::rotate ? centerInitialSize_.second
                                                  : centerInitialSize_.first,
              GizmoPart::center);
   checkGizmo(gizmoSide_, sideInitialSize_, GizmoPart::hotizontal);
   checkGizmo(gizmoUp_, upInitialSize_, GizmoPart::vertical);

   mouseOnGizmo_ = gizmoTouched;
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