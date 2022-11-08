#pragma once

#include "RendererAPI.hpp"
#include "VertexArray.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace looper {

class RenderCommand
{
 public:
   static void
   Init()
   {
      s_RendererAPI->Init();
   }

   static void
   SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
   {
      s_RendererAPI->SetViewport(x, y, width, height);
   }

   static void
   SetClearColor(const glm::vec4& color)
   {
      s_RendererAPI->SetClearColor(color);
   }

   static void
   Clear()
   {
      s_RendererAPI->Clear();
   }

   static void
   DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray, uint32_t count = 0)
   {
      s_RendererAPI->DrawIndexed(vertexArray, count);
   }

   static void
   DrawLines(uint32_t count)
   {
      s_RendererAPI->DrawLines(count);
   }

 private:
   // NOLINTNEXTLINE
   static inline std::unique_ptr< RendererAPI > s_RendererAPI = RendererAPI::Create();
};

} // namespace looper
