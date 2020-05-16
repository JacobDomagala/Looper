#pragma once

#include <memory>
#include <string>
#include <vector>

namespace dgame {

// Base class for all objects located in the game (player, enemy, light source, particle emitter etc.
class Object
{
 public:
   using ID = int32_t;
   using VectorPtr = std::vector< std::shared_ptr< Object > >;

 public:
   enum class TYPE
   {
      ENEMY,
      PLAYER,
      ANIMATION_POINT,
      PATHFINDER_NODE
   };

   Object(TYPE type);
   virtual ~Object() = default;

   void SetType(TYPE);

   TYPE
   GetType() const;

   std::string
   GetTypeString() const;

   ID
   GetID() const;

 protected:
   TYPE m_type;

   ID m_id;

   static inline ID s_currentID = 0;
};

} // namespace dgame