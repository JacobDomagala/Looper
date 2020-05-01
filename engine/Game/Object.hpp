#pragma once

#include <memory>
#include <string>
#include <vector>

class Object
{
 public:
   using VectorPtr = std::vector< std::shared_ptr< Object > >;

 public:
   enum class TYPE
   {
      ENEMY,
      PLAYER,
      ANIMATION_POINT
   };

   // Constructors and destructors
   Object(TYPE type);
   virtual ~Object() = default;

   void SetType(TYPE);

   TYPE
   GetType() const;

   std::string
   GetTypeString() const;

   int
   GetID() const;

 protected:
   TYPE m_type;

   int m_id;

   static inline int s_currentID = 0;
};