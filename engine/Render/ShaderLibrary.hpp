#pragma once

#include "Logger.hpp"
#include "Shader.hpp"

#include <glad/glad.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace dgame {

class ShaderLibrary
{
 public:

   static std::shared_ptr< Shader >
   GetShader(const std::string& shaderName);

   static void
   Clear();

 private:
   static void
   LoadShader(const std::string& shaderName);

   // NOLINTBEGIN
   static inline std::unordered_map< std::string, std::shared_ptr< Shader > > s_loadedShaders = {};
   static inline Logger s_logger = Logger("ShaderLibrary");
   // NOLINTEND
};

} // namespace dgame