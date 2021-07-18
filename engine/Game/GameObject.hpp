#pragma once

#include "Common.hpp"
#include "Object.hpp"
#include "Shader.hpp"
#include "Sprite.hpp"

#include <deque>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace dgame {

class Application;
class Game;
class Window;

class GameObject : public Object
{
 public:
   // Constructors and destructors
   GameObject(Application& game, const glm::vec2& localPosition, const glm::ivec2& size,
              const std::string& sprite, Object::TYPE type);
   ~GameObject() override = default;

   virtual void Hit(int32_t)
   {
   }

   virtual bool
   Visible() const;

   // SETERS
   virtual void
   SetColor(const glm::vec3& color);

   virtual void
   SetCenteredLocalPosition(const glm::ivec2& pos);

   virtual void
   SetLocalPosition(const glm::ivec2& position);

   virtual void
   SetGlobalPosition(const glm::vec2& position);

   virtual void
   SetShaders(const std::string& shader);

   virtual void
   SetName(const std::string& name);

   virtual void
   SetSize(const glm::vec2& newSize);

   // GETERS

   // Get size of object
   virtual glm::ivec2
   GetSize() const;

   // Get cenetered position in local(level wise) coords
   virtual glm::ivec2
   GetCenteredLocalPosition() const;

   // Get centered position in global(OpenGL) coords
   virtual glm::vec2
   GetCenteredGlobalPosition() const;

   // Get position in global (OpenGL) coords
   virtual glm::vec2
   GetGlobalPosition() const;

   // Get position in local (level wise) coords
   virtual glm::ivec2
   GetLocalPosition() const;

   virtual bool
   CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const;

   // Get position in (0,0) to (WIDTH, HEIGHT) screen coords (0,0 BEING TOP LEFT CORNER)
   virtual glm::vec2
   GetScreenPositionPixels() const;

   virtual const Sprite&
   GetSprite() const;

   virtual Sprite&
   GetSprite();

   std::string
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
   Move(const glm::vec2& moveBy, bool isCameraMovement = true);

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
   bool
   GetHasCollision() const;

   std::vector< std::pair< int32_t, int32_t > >
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
      glm::vec2 m_globalPosition;

      // center of global's position (in OpenGL coords)
      glm::vec2 m_centeredGlobalPosition;

      // local position (map coords)
      glm::ivec2 m_localPosition;

      // center of local's position (map coords)
      glm::ivec2 m_centeredLocalPosition;

      // should this object be visible
      bool m_visible;

      // matrices for transforming object
      glm::mat4 m_translateMatrix;
      glm::vec2 m_translateVal;
      glm::mat4 m_rotateMatrix;
      glm::mat4 m_scaleMatrix;

      std::vector< std::pair< int32_t, int32_t > > m_occupiedNodes;
   };

   std::deque< State > m_statesQueue;
   State m_currentState;

   Application& m_appHandle;

   TYPE m_type;

   bool m_hasCollision = false;

   // object's sprite
   Sprite m_sprite;
   std::string m_shaderName;

   // byte array of sprite used for collision
   byte_vec4* m_collision;

   std::string m_name = "DummyName";
   int m_id;

   static inline int s_currentID = 0;
};

} // namespace dgame