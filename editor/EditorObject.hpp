#pragma once

#include "Common.hpp"
#include "GameObject.hpp"
#include "Object.hpp"
#include "Shader.hpp"
#include "Sprite.hpp"

#include <glm/glm.hpp>

namespace looper {

class Editor;

class EditorObject
{
 public:
   EditorObject(Editor& editor, const glm::vec2& positionOnMap, const glm::ivec2& size,
                const std::string& sprite, Object::ID linkedObject);

   [[nodiscard]] bool
   Visible() const;

   // SETERS
   void
   SetColor(const glm::vec3& color);

   void
   SetIsBackground(bool isBackground);

   [[nodiscard]] bool
   GetIsBackground() const;

   void
   SetPosition(const glm::vec2& position);

   void
   SetShaders(const Shader& program);

   void
   SetName(const std::string& name);

   void
   SetVisible(bool visible);

   [[nodiscard]] bool
   IsVisible() const;

   // Get size of object
   [[nodiscard]] glm::ivec2
   GetSize() const;

   // Get centered position in global(OpenGL) coords
   [[nodiscard]] glm::vec2
   GetCenteredPosition() const;

   // Get position in global (OpenGL) coords
   [[nodiscard]] glm::vec2
   GetPosition() const;

   [[nodiscard]] bool
   CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const;

   // Get position in (0,0) to (WIDTH, HEIGHT) screen coords (0,0 BEING TOP LEFT CORNER)
   [[nodiscard]] glm::vec2
   GetScreenPositionPixels() const;

   [[nodiscard]] const Sprite&
   GetSprite() const;

   Sprite&
   GetSprite();

   [[nodiscard]] std::string
   GetName() const;

   [[nodiscard]] Object::ID
   GetLinkedObjectID() const;

   void
   DeleteLinkedObject();

   // Create sprite with default texture
   void
   CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
                const glm::ivec2& size = glm::ivec2(10, 10));

   // Create sprite with texture from 'fileName'
   void
   CreateSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
                        const glm::ivec2& size = glm::ivec2(16, 16),
                        const std::string& fileName = "Default.png");

   // Move object by 'moveBy'
   void
   Move(const glm::vec2& moveBy);

   void
   Scale(const glm::vec2& scaleVal, bool cumulative = false);

   void
   Rotate(float angle, bool cumulative = false);

   // Render object
   void
   Render();

   void
   Update(bool isReverse);

   void
   SetObjectSelected();

   // Only used by editor when unselected by user
   void
   SetObjectUnselected();

 private:
   Editor& m_editor;

   // global position (in OpenGL coords)
   glm::vec2 m_position;

   // center of global's position (in OpenGL coords)
   glm::vec2 m_centeredPosition;

   // should this object be visible
   bool m_visible = false;

   std::string m_name;

   // Linked object's ID
   Object::ID m_objectID = Object::INVALID_ID;
   bool m_hasLinkedObject = false;


   bool m_selected = false;
   bool m_isBackground = false;

   // object's sprite
   Sprite m_sprite;
};

} // namespace looper
