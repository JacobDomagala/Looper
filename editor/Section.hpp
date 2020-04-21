#pragma once

#include <nanogui/button.h>
#include <vector>
#include <string>

class Section : public nanogui::Button
{
 public:
   Section(nanogui::Widget* parent, const std::string& name, bool activeByDefault = true);

   void
   AddWidget(nanogui::Widget* widget);

   void
   RemoveWidget(nanogui::Widget* widget);

   void
   setActive(bool active);

   void
   UpdateWidgets(bool visible);

 private:
   bool m_sectionActive = true;
   std::vector< nanogui::Widget* > m_widgets;
};
