#pragma once

#include "Common.hpp"
#include "Shaders.hpp"
#include "Sprite.hpp"

#include <GL/glew.h>
#include <deque>
#include <glm/glm.hpp>

class Context;
class Game;
class Window;

class GameObject
{
 public:
   // Constructors and destructors
   GameObject(Context& game, const glm::vec2& localPosition, const glm::ivec2& size, const std::string& sprite);
   virtual ~GameObject() = default;

   virtual void Hit(int32_t) = 0;

   virtual void
   DealWithPlayer() = 0;

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
   SetShaders(const Shaders& program);

   // GETERS

   // Get size of object
   virtual glm::vec2
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
   
   // Get position in (0,0) to (WIDTH, HEIGHT) screen coords (0,0 BEING TOP LEFT CORNER)
   virtual glm::vec2
   GetScreenPositionPixels() const; 
   
   virtual const Sprite&
   GetSprite() const;

   // Create sprite with default texture
   virtual void
   CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::ivec2& size = glm::ivec2(10, 10));

   // Create sprite with texture from 'fileName'
   virtual void
   CreateSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::ivec2& size = glm::ivec2(16, 16),
                        const std::string& fileName = "Default.png");

   // Move object by 'moveBy'
   virtual void
   Move(const glm::vec2& moveBy, bool isCameraMovement = true);

   // Render object
   virtual void
   Render(const glm::mat4& window, const Shaders& program);

   virtual void
   Update(bool isReverse);

 protected:
   // should be overriden by derrived class
   // used by GameObject::Update
   virtual void
   UpdateInternal(bool isReverse) = 0;

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
   };

   std::deque< State > m_statesQueue;
   State m_currentState;

   Context& m_contextHandle;

   // object's sprite
   Sprite m_sprite;

   // object's shaders
   Shaders m_program;

   // byte array of sprite used for collision
   byte_vec4* m_collision;
};
