#pragma once

#include "Common.hpp"
#include "Shaders.hpp"
#include "Sprite.hpp"

#include <GL/glew.h>
#include <deque>
#include <glm/glm.hpp>

namespace dgame {

class Editor;
class Object;

class EditorObject
{
 public:
   // Constructors and destructors
   EditorObject(Editor& editor, const glm::vec2& localPosition, const glm::ivec2& size, const std::string& sprite,
                std::shared_ptr< Object > linkedObject);
   ~EditorObject() = default;
   EditorObject(EditorObject&&) = default;

   bool
   Visible() const;

   // SETERS
   void
   SetColor(const glm::vec3& color);

   void
   SetCenteredLocalPosition(const glm::ivec2& pos);

   void
   SetLocalPosition(const glm::ivec2& position);

   void
   SetGlobalPosition(const glm::vec2& position);

   void
   SetShaders(const Shaders& program);

   void
   SetName(const std::string& name);

   void
   SetVisible(bool visible);

   bool
   GetVisible();

   // GETERS

   // Get size of object
   glm::ivec2
   GetSize() const;

   // Get cenetered position in local(level wise) coords
   glm::ivec2
   GetCenteredLocalPosition() const;

   // Get centered position in global(OpenGL) coords
   glm::vec2
   GetCenteredGlobalPosition() const;

   // Get position in global (OpenGL) coords
   glm::vec2
   GetGlobalPosition() const;

   // Get position in local (level wise) coords
   glm::ivec2
   GetLocalPosition() const;

   bool
   CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const;

   // Get position in (0,0) to (WIDTH, HEIGHT) screen coords (0,0 BEING TOP LEFT CORNER)
   glm::vec2
   GetScreenPositionPixels() const;

   const Sprite&
   GetSprite() const;

   Sprite&
   GetSprite();

   std::string
   GetName() const;

   std::shared_ptr< Object >
   GetLinkedObject();

   // Create sprite with default texture
   void
   CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::ivec2& size = glm::ivec2(10, 10));

   // Create sprite with texture from 'fileName'
   void
   CreateSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::ivec2& size = glm::ivec2(16, 16),
                        const std::string& fileName = "Default.png");

   // Move object by 'moveBy'
   void
   Move(const glm::vec2& moveBy, bool isCameraMovement = true);

   void
   Scale(const glm::vec2& scaleVal, bool cumulative = false);

   void
   Rotate(float angle, bool cumulative = false);

   // Render object
   void
   Render(Shaders& program);

   void
   Update(bool isReverse);

   void
   SetObjectSelected();

   // Only used by editor when unselected by user
   void
   SetObjectUnselected();

 private:
   // global position (in OpenGL coords)
   glm::vec2 m_globalPosition;

   // center of global's position (in OpenGL coords)
   glm::vec2 m_centeredGlobalPosition;

   // local position (map coords)
   glm::ivec2 m_localPosition;

   // center of local's position (map coords)
   glm::ivec2 m_centeredLocalPosition;

   // should this object be visible
   bool m_visible = false;

   // matrices for transforming object
   glm::mat4 m_translateMatrix;
   glm::vec2 m_translateVal;
   glm::mat4 m_rotateMatrix;
   glm::mat4 m_scaleMatrix;

   std::string m_name;

   int m_objectID = -1;
   std::shared_ptr< Object > m_linkedObject = nullptr;
   Editor& m_editor;

   bool m_selected = false;

   // object's sprite
   Sprite m_sprite;
};

} // namespace dgame