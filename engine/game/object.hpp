#pragma once

#include "types.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace looper {

// Base class for all objects located in the game (player, enemy, light source, particle emitter
// etc.
// NOLINTNEXTLINE
class Object
{
 public:
   using ID = uint64_t;
   using VectorPtr = std::vector< std::shared_ptr< Object > >;

   // NOLINTNEXTLINE
   static constexpr ID INVALID_ID = static_cast< ID >(~0);
   static constexpr uint32_t TYPE_NUM_BITS = 32;

   explicit Object(ObjectType type);
   virtual ~Object() = default;

   static ObjectType
   GetTypeFromString(const std::string& stringType);

   static ObjectType
   GetTypeFromID(ID id);

   static std::string
   GetTypeString(ID id);

   void SetType(ObjectType);

   [[nodiscard]] ObjectType
   GetType() const;

   [[nodiscard]] std::string
   GetTypeString() const;

   [[nodiscard]] ID
   GetID() const;

   void
   SetID(ID);

 protected:
   ObjectType m_type;

   ID m_id;

   // NOLINTNEXTLINE
   static inline std::unordered_map< std::string, ObjectType > s_map = {
      {"Enemy", ObjectType::ENEMY},
      {"Player", ObjectType::PLAYER},
      {"Object", ObjectType::OBJECT},
      {"Animation Point", ObjectType::ANIMATION_POINT},
      {"Editor Object", ObjectType::EDITOR_OBJECT},
      {"Pathfinder Node", ObjectType::PATHFINDER_NODE}};
   static inline ID s_currentID = 0; // NOLINT
};

inline bool
operator==(const Object& left, const Object& right)
{
   return left.GetID() == right.GetID();
}

} // namespace looper