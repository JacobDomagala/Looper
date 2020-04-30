#pragma once

#include <nanogui/button.h>
#include <vector>
#include <string>

class Editor;

class Section : public nanogui::Button
{
 public:
   Section(nanogui::Widget* parent, Editor& editor, const std::string& name, bool activeByDefault = true);

   void
   AddWidget(nanogui::Widget* widget);

   void
   RemoveWidget(nanogui::Widget* widget);

   void
   setActive(bool active);

   void
   UpdateWidgets(bool visible);

 protected:
   Editor& m_editor;
   bool m_sectionActive = true;
   std::vector< nanogui::Widget* > m_widgets;
};
