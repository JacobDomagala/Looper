#include "object.hpp"

#include <unordered_map>

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

      case ObjectType::EDITOR_OBJECT: {
         typeStr = "Editor Object";
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

// NOLINTNEXTLINE 
std::unordered_map< std::string, ObjectType > typesMap = {
   {"Enemy", ObjectType::ENEMY},
   {"Player", ObjectType::PLAYER},
   {"Object", ObjectType::OBJECT},
   {"Animation Point", ObjectType::ANIMATION_POINT},
   {"Editor Object", ObjectType::EDITOR_OBJECT},
   {"Pathfinder Node", ObjectType::PATHFINDER_NODE}};

} // namespace

ObjectType
Object::GetTypeFromString(const std::string& stringType)
{
   return typesMap[stringType];
}

Object::Object(ObjectType type) : type_(type)
{
   Setup(type);
}

void
Object::Setup(ObjectType type)
{
   type_ = type;

   // First 32 bits are for ids, the other are for type storage
   auto type_val = static_cast< ID >(type) << TYPE_NUM_BITS;
   id_ = type_val + currentID_;

   ++currentID_;
}

void
Object::SetType(ObjectType newType)
{
   type_ = newType;
}

ObjectType
Object::GetType() const
{
   return type_;
}

std::string
Object::GetTypeString() const
{
   return TypeToString(type_);
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
   else if (type_part == static_cast< ID >(ObjectType::EDITOR_OBJECT))
   {
      type = ObjectType::EDITOR_OBJECT;
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
   return id_;
}

void
Object::SetID(Object::ID id)
{
   id_ = id;
}

} // namespace looper