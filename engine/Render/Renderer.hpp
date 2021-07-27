#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace dgame {

class Texture;
class Camera;

enum class PrimitiveType
{
   QUAD,
   LINE
};

class Renderer
{
 public:
   static void
   Init();

   static void
   Shutdown();

   static void
   BeginScene(const Camera& camera);

   static void
   EndScene();

   // Primitives
   static void
   DrawQuad(const glm::vec2& position, const glm::vec2& size, float radiansRotation,
            const std::shared_ptr< Texture >& texture = std::make_shared< Texture >(),
            float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

   static void
   DrawLine(const glm::vec2& startPosition, const glm::vec2& endPosition, const glm::vec4& color);

 private:
   static void
   SendData(PrimitiveType type);

   static void
   Flush(PrimitiveType type);

   static void
   FlushAndReset(PrimitiveType type);
};

} // namespace dgame