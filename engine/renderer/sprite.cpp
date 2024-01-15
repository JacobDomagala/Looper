#include "sprite.hpp"
#include "application.hpp"
#include "renderer.hpp"
#include "renderer/camera/collision_camera.hpp"
#include "texture.hpp"

#include <glm/gtx/transform.hpp>

namespace looper::renderer {

void
Sprite::ClearData() const
{
   const auto transformMat = ComputeModelMat();

   renderer::SubmitMeshData(static_cast< uint32_t >(renderInfo_.idx), textures_, transformMat,
                            {0.0f, 0.0f, 0.0f, 0.0f});

   renderer::MeshDeleted(renderInfo_);
}


glm::mat4
Sprite::ComputeModelMat() const
{
   return glm::translate(glm::mat4(1.0f), glm::vec3{currentState_.translateVal_, 0.0f})
          * glm::rotate(glm::mat4(1.0f), currentState_.angle_, {0.0f, 0.0f, 1.0f})
          * glm::scale(glm::mat4(1.0f), {size_ * currentState_.modifiers.scale, 1.0f});
}

void
Sprite::ChangeRenderLayer(int32_t newLayer)
{
   renderer::MeshDeleted(renderInfo_);

   for (auto& vertex : vertices_)
   {
      vertex.m_position.z = static_cast< float >(newLayer) / 20.0f;
   }

   const auto transformMat = ComputeModelMat();

   renderInfo_ = MeshLoaded(vertices_, textures_, transformMat, currentState_.color_);
   changed_ = true;
}

void
Sprite::SetSprite(const glm::vec3& position, const glm::vec2& size)
{
   // m_texture = std::make_shared< Texture >();
   // m_texture->CreateColorTexture(size, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

   currentState_.currentPosition_ = position;
   initialPosition_ = position;
   currentState_.translateVal_ = position;
   size_ = size;
   currentState_.angle_ = 0.0f;
   currentState_.scaleVal_ = glm::vec2(1.0f, 1.0f);
   currentState_.color_ = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

void
Sprite::SetSpriteTextured(const glm::vec3& position, const glm::vec2& size,
                          const std::string& fileName)
{
   changed_ = true;

   initialPosition_ = position;
   currentState_.currentPosition_ = position;
   size_ = size;
   initialSize_ = size;

   currentState_.translateVal_ = position;
   currentState_.angle_ = 0.0f;
   currentState_.scaleVal_ = glm::vec2(1.0f, 1.0f);
   currentState_.color_ = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);


   /*                          y -1
                                ^
                                |
       x (0) [-0.5f, 0.5f]      |     x (1) [0.5f, 0.5f]
                                |
     -------------------------------------------------------->  x +1
                                |
                                |
        x (3) [-0.5f, -0.5f]    |     x (2) [0.5f, -0.5f]

   */
   vertices_ = std::vector< renderer::Vertex >{
      {glm::vec3{-0.5f, 0.5f, position.z}, glm::vec3{0.0f, 0.0f, 1.0f}},
      {glm::vec3{0.5f, 0.5f, position.z}, glm::vec3{1.0f, 0.0f, 1.0f}},
      {glm::vec3(0.5f, -0.5f, position.z), glm::vec3{1.0f, 1.0f, 1.0f}},
      {glm::vec3{-0.5f, -0.5f, position.z}, glm::vec3{0.0f, 1.0f, 1.0f}}};

   const auto transformMat = ComputeModelMat();
   ComputeBoundingBox();

   textures_ = {TextureLibrary::GetTexture(fileName)->GetID(),
                TextureLibrary::GetTexture("white.png")->GetID(),
                TextureLibrary::GetTexture(fileName)->GetID(),
                TextureLibrary::GetTexture(fileName)->GetID()};

   renderInfo_ = MeshLoaded(vertices_, textures_, transformMat, currentState_.color_);
}

void
Sprite::SetSpriteTextured(const glm::vec2& position, const glm::vec2& size,
                          const std::string& fileName)
{
   SetSpriteTextured(glm::vec3{position, 0.0f}, size, fileName);
}

void
Sprite::Update(bool isReverse)
{
   if (isReverse)
   {
      currentState_ = statesQueue_.GetLastState();

      changed_ = true;
   }
   else
   {
      statesQueue_.PushState(currentState_);
   }
}

void
Sprite::Show()
{
   SetColor({currentState_.color_.x, currentState_.color_.y, currentState_.color_.z, 1.0f});
}

void
Sprite::Hide()
{
   SetColor({currentState_.color_.x, currentState_.color_.y, currentState_.color_.z, 0.0f});
}

void
Sprite::Render()
{
   if (changed_)
   {
      const auto transformMat = ComputeModelMat();
      ComputeBoundingBox();

      renderer::SubmitMeshData(static_cast< uint32_t >(renderInfo_.idx), textures_, transformMat,
                               currentState_.color_);

      changed_ = false;
   }
}

glm::vec3
Sprite::GetPosition() const
{
   return currentState_.currentPosition_;
}

glm::vec2
Sprite::GetSize() const
{
   return size_;
}

void
Sprite::SetSize(const glm::vec2& newSize)
{
   size_ = newSize;
   changed_ = true;
}

glm::vec2
Sprite::GetOriginalSize() const
{
   return size_;
}

std::string
Sprite::GetTextureName() const
{
   return renderer::TextureLibrary::GetTexture(textures_.at(0))->GetName();
}

glm::vec2
Sprite::GetTranslation() const
{
   return currentState_.translateVal_;
}

float
Sprite::GetRotation(RotationType type) const
{
   return type == RotationType::DEGREES ? glm::degrees(currentState_.angle_) : currentState_.angle_;
}

glm::vec2&
Sprite::GetScale()
{
   return currentState_.scaleVal_;
}

const glm::vec2&
Sprite::GetScale() const
{
   return currentState_.scaleVal_;
}

float&
Sprite::GetUniformScaleValue()
{
   return currentState_.uniformScaleValue_;
}

void
Sprite::SetColor(const glm::vec4& color)
{
   if (currentState_.color_ != color)
   {
      currentState_.color_ = color;
      changed_ = true;
   }
}

void
Sprite::SetTextureFromFile(const std::string& filePath)
{
   textures_[0] = renderer::TextureLibrary::GetTexture(filePath)->GetID();
   changed_ = true;
}

void
Sprite::SetTranslateValue(const glm::vec2& translateBy)
{
   currentState_.currentPosition_ = initialPosition_ + glm::vec3{translateBy, 0.0f};
   currentState_.translateVal_ = currentState_.currentPosition_;

   changed_ = true;
}

void
Sprite::SetInitialPosition(const glm::vec3& globalPosition)
{
   initialPosition_ = globalPosition;
   currentState_.currentPosition_ = globalPosition;
   currentState_.translateVal_ = globalPosition;

   changed_ = true;
}

void
Sprite::SetTextureID(TextureType type, TextureID newID)
{
   switch (type)
   {
      case TextureType::DIFFUSE_MAP: {
         textures_[0] = newID;
      }
      break;
      case TextureType::MASK_MAP: {
         textures_[1] = newID;
      }
      break;
      case TextureType::NORMAL_MAP: {
         textures_[2] = newID;
      }
      break;
      case TextureType::SPECULAR_MAP: {
         textures_[3] = newID;
      }
      break;
      default: {
      }
   }

   changed_ = true;
}

const renderer::Texture*
Sprite::GetTexture() const
{
   return renderer::TextureLibrary::GetTexture(textures_.at(0));
}

void
Sprite::Rotate(float angle, RotationType type)
{
   currentState_.angle_ = type == RotationType::DEGREES ? glm::radians(angle) : angle;
   currentState_.angle_ =
      glm::clamp(currentState_.angle_, glm::radians(-360.0f), glm::radians(360.0f));

   changed_ = true;
}

void
Sprite::RotateCumulative(float angle, RotationType type)
{
   currentState_.angle_ += type == RotationType::DEGREES ? glm::radians(angle) : angle;
   currentState_.angle_ =
      glm::clamp(currentState_.angle_, glm::radians(-360.0f), glm::radians(360.0f));

   changed_ = true;
}

void
Sprite::Scale(const glm::vec2& scaleValue)
{
   size_ = scaleValue;
   size_ = glm::clamp(size_, SCALE_RANGE.first, SCALE_RANGE.second);

   changed_ = true;
}

void
Sprite::ScaleCumulative(const glm::vec2& scaleValue)
{
   size_ += scaleValue;
   size_ = glm::clamp(size_, SCALE_RANGE.first, SCALE_RANGE.second);

   changed_ = true;
}

void
Sprite::Translate(const glm::vec2& translateValue)
{
   currentState_.currentPosition_ += glm::vec3{translateValue, 0.0f};
   currentState_.translateVal_ += translateValue;

   changed_ = true;
}

void
Sprite::SetModifiers(const Modifiers& mod)
{
   currentState_.modifiers = mod;
}

void
Sprite::ScaleUniformly(const float scaleValue)
{
   currentState_.uniformScaleValue_ = scaleValue;
   /*  currentState_.scaleVal_ += glm::vec2(scaleValue, scaleValue);
     currentState_.scaleVal_.x = glm::clamp(currentState_.scaleVal_.x, m_scaleRange.first,
     m_scaleRange.second); currentState_.scaleVal_.y = glm::clamp(currentState_.scaleVal_.y,
     m_scaleRange.first, m_scaleRange.second);*/

   changed_ = true;
}

void
Sprite::ComputeBoundingBox()
{
   const auto transformMat =
      glm::translate(glm::mat4(1.0f), glm::vec3{currentState_.translateVal_, 0.0f})
      * glm::rotate(glm::mat4(1.0f), currentState_.angle_, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {size_, 1.0f});

   boundingBox_.at(1) = transformMat * glm::vec4(vertices_[0].m_position, 1.0f);
   boundingBox_.at(2) = transformMat * glm::vec4(vertices_[3].m_position, 1.0f);
   boundingBox_.at(0) = transformMat * glm::vec4(vertices_[1].m_position, 1.0f);
   boundingBox_.at(3) = transformMat * glm::vec4(vertices_[2].m_position, 1.0f);
}

std::array< glm::vec2, 4 >
Sprite::GetTransformedRectangle() const
{
   return boundingBox_;
}

bool
Sprite::CheckIfCollidedScreenPosion(const glm::vec3& cameraPosition,
                                    const glm::vec2& globalPosition) const
{
   const renderer::CollisionCamera camera(cameraPosition, this);
   return camera.CheckCollision(globalPosition);
}

} // namespace looper::renderer
