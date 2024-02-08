#include "gizmo.hpp"
#include "types.hpp"

namespace looper {

void
Gizmo::Initialize()
{
   gizmoCenter_.SetSpriteTextured(glm::vec3{0.0f, 0.0f, renderer::LAYER_0}, {16, 16},
                                  "centered_move.png", renderer::SpriteType::alwaysOnTop);
   gizmoCenter_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

   gizmoUp_.SetSpriteTextured(glm::vec3{0.0f, 0.0f, renderer::LAYER_0}, {96, 32}, "arrow.png",
                              renderer::SpriteType::alwaysOnTop);
   gizmoUp_.Rotate(90.0f, renderer::RotationType::degrees);
   gizmoUp_.SetColor({0.0f, 1.0f, 0.0f, 1.0f});

   gizmoSide_.SetSpriteTextured(glm::vec3{0.0f, 0.0f, renderer::LAYER_0}, {96, 32}, "arrow.png",
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

   gizmoCenter_.SetInitialPosition(glm::vec3{centeredPos, renderer::LAYER_1});

   AdjustSize();
}

void
Gizmo::Move(const glm::vec2& moveBy)
{
   gizmoCenter_.Translate(moveBy);
   gizmoUp_.Translate(moveBy);
   gizmoSide_.Translate(moveBy);
}

void
Gizmo::Zoom(int32_t zoomVal)
{
   zoomLevel_ = glm::min(zoomLevel_ + zoomVal, 18);

   AdjustSize();
}

void
Gizmo::AdjustSize()
{
   float scaleFactor = 1.0f;
   if (zoomLevel_ >= 0)
   {
      scaleFactor = 20.0f / (20.0f - static_cast< float >(zoomLevel_));
   }

   gizmoCenter_.Scale(
      (currentState_ == GizmoState::rotate ? centerInitialSize_.second : centerInitialSize_.first)
      * scaleFactor);
   centerCurrentSize_ = {centerInitialSize_.first * scaleFactor,
                         centerInitialSize_.second * scaleFactor};

   upCurrentSize_ = glm::vec2{upInitialSize_.x * scaleFactor, upInitialSize_.y * scaleFactor};
   gizmoUp_.Scale(upCurrentSize_);

   sideCurrentSize_ = glm::vec2{sideInitialSize_.x * scaleFactor, sideInitialSize_.y * scaleFactor};
   gizmoSide_.Scale(sideCurrentSize_);

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
              currentState_ == GizmoState::rotate ? centerCurrentSize_.second
                                                  : centerCurrentSize_.first,
              GizmoPart::center);
   checkGizmo(gizmoSide_, sideCurrentSize_, GizmoPart::hotizontal);
   checkGizmo(gizmoUp_, upCurrentSize_, GizmoPart::vertical);

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

   AdjustSize();
}
void
Gizmo::SwitchToRotate()
{
   currentState_ = GizmoState::rotate;

   gizmoSide_.Hide();
   gizmoUp_.Hide();

   gizmoCenter_.SetTextureID(renderer::TextureType::DIFFUSE_MAP,
                             renderer::TextureLibrary::GetTexture("rotate.png")->GetID());

   gizmoCenter_.Rotate(currentRotation_);

   AdjustSize();
}
void
Gizmo::SwitchToTranslate()
{
   currentState_ = GizmoState::translate;

   AdjustSize();
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