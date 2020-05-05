#pragma once

#include "GuiAPI.hpp"

#include <string>
#include <vector>

namespace dgame {

class Editor;

class Section : public Button
{
 public:
   Section(Widget* parent, Editor& editor, const std::string& name, bool activeByDefault = true);

   void
   AddWidget(Widget* widget);

   void
   RemoveWidget(Widget* widget);

   void
   setActive(bool active);

   void
   UpdateWidgets(bool visible);

 protected:
   Editor& m_editor;
   bool m_sectionActive = true;
   std::vector< Widget* > m_widgets;
};

} // namespace dgame