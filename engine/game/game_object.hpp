#pragma once

#include "common.hpp"
#include "object.hpp"
// #include "Shader.hpp"
#include "renderer/sprite.hpp"

#include <deque>
#include <glm/glm.hpp>

namespace looper {

class Application;
class Game;

class GameObject : public Object
{
 public:
   // Constructors and destructors
   GameObject(Application& application, const glm::vec2& position, const glm::ivec2& size,
              const std::string& sprite, Object::TYPE type);

   virtual void Hit(int32_t /*dmg*/)
   {
   }

   [[nodiscard]] virtual bool
   Visible() const;

   // SETERS
   virtual void
   SetColor(const glm::vec3& color);

   virtual void
   SetPosition(const glm::vec2& position);

   virtual void
   SetShaders(const std::string& shader);

   virtual void
   SetName(const std::string& name);

   virtual void
   SetSize(const glm::vec2& newSize);

   // GETERS

   // Get size of object
   [[nodiscard]] virtual glm::ivec2
   GetSize() const;

   // Get centered position in global(OpenGL) coords
   [[nodiscard]] virtual glm::vec2
   GetCenteredPosition() const;

   // Get position in global (OpenGL) coords
   [[nodiscard]] virtual glm::vec2
   GetPosition() const;

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
   CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
                const glm::ivec2& size = glm::ivec2(10, 10));

   // Create sprite with texture from 'fileName'
   virtual void
   CreateSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
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

   [[nodiscard]] std::vector< Tile_t >
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
      glm::vec2 m_position;

      // center of global's position (in OpenGL coords)
      glm::vec2 m_centeredPosition;

      // should this object be visible
      bool m_visible;

      // matrices for transforming object
      glm::mat4 m_translateMatrix;
      glm::vec2 m_translateVal;
      glm::mat4 m_rotateMatrix;
      glm::mat4 m_scaleMatrix;

      std::vector< Tile_t > m_occupiedNodes;
   };

   std::deque< State > m_gameObjectStatesQueue;
   State m_currentGameObjectState;

   Application& m_appHandle;

   bool m_hasCollision = false;

   // object's sprite
   renderer::Sprite m_sprite;
   std::string m_shaderName;

   std::string m_name = "DummyName";
};

} // namespace looper