#include "Object.hpp"

namespace dgame {

Object::Object(TYPE type)
{
   m_type = type;
   m_id = s_currentID;

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
   std::string type = "";

   switch (m_type)
   {
      case TYPE::ENEMY: {
         type = "Enemy";
      }
      break;

      case TYPE::PLAYER: {
         type = "Player";
      }
      break;

      case TYPE::ANIMATION_POINT: {
         type = "Animation Point";
      }
      break;

      case TYPE::PATHFINDER_NODE: {
         type = "Pathfinder Node";
      }
      break;

      default: {
         type = "No Type";
      }
   }

   return type;
}

int
Object::GetID() const
{
   return m_id;
}

} // namespace dgame