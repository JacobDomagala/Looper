#include "ShaderLibrary.hpp"

namespace dgame {

std::shared_ptr< Shader >
ShaderLibrary::GetShader(const std::string& shaderName)
{
   if (s_loadedShaders.find(shaderName) == s_loadedShaders.end())
   {
      s_logger.Log(Logger::TYPE::INFO, "Shader: {} not found in library. Loading it", shaderName);
      LoadShader(shaderName);
   }

   return s_loadedShaders[shaderName];
}

void
ShaderLibrary::Clear()
{
   s_loadedShaders.clear();
}

void
ShaderLibrary::LoadShader(const std::string& shaderName)
{
   s_loadedShaders[shaderName] = std::make_shared< Shader >(shaderName);
}

} // namespace dgame