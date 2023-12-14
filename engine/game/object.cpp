#include "object.hpp"

namespace looper {
namespace {
std::string
TypeToString(ObjectType type)
{
   std::string typeStr;

   switch (type)
   {
      case ObjectType::ENEMY: {
         typeStr = "Enemy";
      }
      break;

      case ObjectType::PLAYER: {
         typeStr = "Player";
      }
      break;

      case ObjectType::OBJECT: {
         typeStr = "Object";
      }
      break;

      case ObjectType::ANIMATION_POINT: {
         typeStr = "Animation Point";
      }
      break;

      case ObjectType::PATHFINDER_NODE: {
         typeStr = "Pathfinder Node";
      }
      break;

      default: {
         typeStr = "No Type";
      }
   }

   return typeStr;
}
} // namespace

ObjectType
Object::GetTypeFromString(const std::string& stringType)
{
   return s_map[stringType];
}

Object::Object(ObjectType type) : m_type(type)
{
   // First 32 bits are for ids, the other are for type storage
   auto type_val = static_cast< ID >(type) << TYPE_NUM_BITS;
   m_id = type_val + s_currentID;

   ++s_currentID;
}

void
Object::SetType(ObjectType newType)
{
   m_type = newType;
}

ObjectType
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

ObjectType
Object::GetTypeFromID(ID id)
{
   // Shift 'id' value to its type part
   const auto type_part = id >> TYPE_NUM_BITS;
   ObjectType type = ObjectType::NONE;

   if (type_part == static_cast< ID >(ObjectType::ENEMY))
   {
      type = ObjectType::ENEMY;
   }
   else if (type_part == static_cast< ID >(ObjectType::PLAYER))
   {
      type = ObjectType::PLAYER;
   }
   else if (type_part == static_cast< ID >(ObjectType::ANIMATION_POINT))
   {
      type = ObjectType::ANIMATION_POINT;
   }
   else if (type_part == static_cast< ID >(ObjectType::PATHFINDER_NODE))
   {
      type = ObjectType::PATHFINDER_NODE;
   }
   else if (type_part == static_cast< ID >(ObjectType::OBJECT))
   {
      type = ObjectType::OBJECT;
   }

   return type;
}

Object::ID
Object::GetID() const
{
   return m_id;
}

void
Object::SetID(Object::ID id)
{
   m_id = id;
}

} // namespace looper