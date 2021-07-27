#include "Object.hpp"

namespace dgame {

static std::string
TypeToString(Object::TYPE type)
{
   std::string typeStr = "";

   switch (type)
   {
      case Object::TYPE::ENEMY: {
         typeStr = "Enemy";
      }
      break;

      case Object::TYPE::PLAYER: {
         typeStr = "Player";
      }
      break;

      case Object::TYPE::OBJECT: {
         typeStr = "Object";
      }
      break;

      case Object::TYPE::ANIMATION_POINT: {
         typeStr = "Animation Point";
      }
      break;

      case Object::TYPE::PATHFINDER_NODE: {
         typeStr = "Pathfinder Node";
      }
      break;

      default: {
         typeStr = "No Type";
      }
   }

   return typeStr;
}

Object::TYPE
Object::GetTypeFromString(const std::string& stringType)
{
   return s_map[stringType];
}

Object::Object(TYPE type)
{
   m_type = type;

   // First 32 bits are for ids, the other are for type storage
   auto type_val = static_cast< ID >(type) << TYPE_NUM_BITS;
   m_id = type_val + s_currentID;

   ++s_currentID;
}

void
Object::SetType(TYPE newType)
{
   m_type = newType;
}

Object::TYPE
Object::GetType() const
{
   return m_type;
}

std::string
Object::GetTypeString() const
{
   return TypeToString(m_type);
}

std::string
Object::GetTypeString(ID id)
{
   return TypeToString(GetTypeFromID(id));
}

Object::TYPE
Object::GetTypeFromID(ID id)
{
   // Shift 'id' value to its type part
   const auto type_part = id >> TYPE_NUM_BITS;

   if (type_part & static_cast< ID >(TYPE::ENEMY))
   {
      return TYPE::ENEMY;
   }
   else if (type_part & static_cast< ID >(TYPE::PLAYER))
   {
      return TYPE::PLAYER;
   }
   else if (type_part & static_cast< ID >(TYPE::ANIMATION_POINT))
   {
      return TYPE::ANIMATION_POINT;
   }
   else if (type_part & static_cast< ID >(TYPE::PATHFINDER_NODE))
   {
      return TYPE::PATHFINDER_NODE;
   }
   else
   {
      return TYPE::NONE;
   }
}

Object::ID
Object::GetID() const
{
   return m_id;
}

} // namespace dgame