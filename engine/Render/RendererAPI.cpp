
#include "RendererAPI.hpp"
#include "OpenGLRendererAPI.hpp"

#include <memory>

namespace dgame {

std::unique_ptr< RendererAPI >
RendererAPI::Create()
{
   switch (s_API)
   {
      case RendererAPI::API::None:
         return nullptr;
      case RendererAPI::API::OpenGL:
         return std::make_unique< OpenGLRendererAPI >();
   }

   return nullptr;
}

} // namespace dgame