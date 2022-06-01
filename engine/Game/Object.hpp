#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dgame {

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

   enum class TYPE
   {
      NONE = 0,
      ENEMY = 1,
      PLAYER = 2,
      OBJECT = 3,
      ANIMATION_POINT = 4,
      PATHFINDER_NODE = 8
   };

   explicit Object(TYPE type);
   virtual ~Object() = default;

   static TYPE
   GetTypeFromString(const std::string& stringType);

   static TYPE
   GetTypeFromID(ID id);

   static std::string
   GetTypeString(ID id);

   void SetType(TYPE);

   [[nodiscard]] TYPE
   GetType() const;

   [[nodiscard]] std::string
   GetTypeString() const;

   [[nodiscard]] ID
   GetID() const;

 protected:
   TYPE m_type;

   ID m_id;

   // NOLINTNEXTLINE
   static inline std::unordered_map< std::string, TYPE > s_map = {
      {"Enemy", Object::TYPE::ENEMY},
      {"Player", Object::TYPE::PLAYER},
      {"Object", Object::TYPE::OBJECT},
      {"Animation Point", Object::TYPE::ANIMATION_POINT},
      {"Pathfinder Node", Object::TYPE::PATHFINDER_NODE}};
   static inline ID s_currentID = 0; // NOLINT
};

inline bool
operator==(const Object& left, const Object& right)
{
   return left.GetID() == right.GetID();
}

} // namespace dgame