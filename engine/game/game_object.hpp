#pragma once

#include "common.hpp"
#include "object.hpp"
#include "state_list.hpp"
// #include "Shader.hpp"
#include "renderer/sprite.hpp"

#include <glm/glm.hpp>

namespace looper {

class Application;
class Game;

class GameObject : public Object
{
 public:
   GameObject(Application* application, const glm::vec3& position, const glm::vec2& size,
              const std::string& sprite, ObjectType type);

   GameObject(Application* application, const glm::vec2& position, const glm::vec2& size,
              const std::string& sprite, ObjectType type);
   GameObject();
   ~GameObject() override;

   void
   Setup(Application* application, const glm::vec3& position, const glm::vec2& size,
         const std::string& sprite, ObjectType type);

   virtual void
   Hit(int32_t /*dmg*/)
   {
   }

   [[nodiscard]] virtual bool
   Visible() const;

   // SETERS
   virtual void
   SetColor(const glm::vec4& color);

   virtual void
   SetPosition(const glm::vec2& position);

   virtual void
   SetName(const std::string& name);

   virtual void
   SetSize(const glm::vec2& newSize);

   // GETERS

   // Get size of object
   [[nodiscard]] virtual glm::ivec2
   GetSize() const;

   // Get centered position in global(Vulkan) coords
   [[nodiscard]] virtual glm::vec2
   GetCenteredPosition() const;

   // Get position in global (Vulkan) coords
   [[nodiscard]] virtual glm::vec2
   GetPosition() const;

   [[nodiscard]] glm::vec2
   GetPreviousPosition() const;

   [[nodiscard]] virtual bool
   CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const;

   // Get position in (0,0) to (WIDTH, HEIGHT) screen coords (0,0 BEING TOP LEFT CORNER)
   [[nodiscard]] virtual glm::vec2
   GetScreenPositionPixels() const;

   [[nodiscard]] virtual const renderer::Sprite&
   GetSprite() const;

   virtual renderer::Sprite&
   GetSprite();

   [[nodiscard]] std::string
   GetName() const;

   // Create sprite with default texture
   virtual void
   CreateSprite(const glm::vec3& position = glm::vec3{},
                const glm::ivec2& size = glm::ivec2(10, 10));

   // Create sprite with texture from 'fileName'
   virtual void
   CreateSpriteTextured(const glm::vec3& position = glm::vec3{},
                        const glm::ivec2& size = glm::ivec2(16, 16),
                        const std::string& fileName = "Default.png");

   // Move object by 'moveBy'
   virtual void
   Move(const glm::vec2& moveBy);

   virtual void
   Scale(const glm::vec2& scaleVal, bool cumulative = false);

   virtual void
   Rotate(float angle, bool cumulative = false);

   // Render object
   virtual void
   Render();

   virtual void
   Update(bool isReverse);

   void
   SetHasCollision(bool hasCollision);

   // Only used by editor when unselected by user
   [[nodiscard]] bool
   GetHasCollision() const;

   [[nodiscard]] std::vector< Tile >
   GetOccupiedNodes() const;

 protected:
   // should be overriden by derrived class
   // used by GameObject::Update
   virtual void
   UpdateInternal(bool /*isReverse*/)
   {
   }

   void
   UpdateCollision();

   Game*
   ConvertToGameHandle();

   struct State
   {
      // global position (in OpenGL coords)
      glm::vec2 position_;

      // center of global's position (in OpenGL coords)
      glm::vec2 centeredPosition_;

      glm::vec2 previousPosition_ = {};

      // should this object be visible
      bool visible_;

      // matrices for transforming object
      glm::mat4 translateMatrix_;
      glm::vec2 translateVal_;
      glm::mat4 rotateMatrix_;
      glm::mat4 scaleMatrix_;

      std::vector< Tile > nodes_;
   };

   StateList< State > gameObjectStatesQueue_;
   State currentGameObjectState_;

   Application* appHandle_;

   bool hasCollision_ = false;
   bool updateCollision_ = false;

   // object's sprite
   renderer::Sprite sprite_;

   std::string name_ = "DummyName";
};

} // namespace looper
