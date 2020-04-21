#include "Section.hpp"

#include <nanogui/entypo.h>

Section::Section(nanogui::Widget* parent, const std::string& name, bool activeByDefault)
   : Button(parent, name, ENTYPO_ICON_CHEVRON_THIN_DOWN)
{
   m_sectionActive = activeByDefault;
   setActive(m_sectionActive);
   setBackgroundColor(nanogui::Color(0.1f, 0.1f, 0.15f, 0.8f));

   auto updateWidgets = [&](bool active) {

   };

   setCallback([&]() {
      if (m_sectionActive)
      {
         this->setIcon(ENTYPO_ICON_CHEVRON_THIN_RIGHT);
         this->UpdateWidgets(false);
      }
      else
      {
         this->setIcon(ENTYPO_ICON_CHEVRON_THIN_DOWN);
         this->UpdateWidgets(true);
      }
   });
}

void
Section::AddWidget(nanogui::Widget* widget)
{
   m_widgets.push_back(widget);
   widget->setVisible(m_sectionActive);
}

void
Section::RemoveWidget(nanogui::Widget* widget)
{
   m_widgets.erase(std::remove(m_widgets.begin(), m_widgets.end(), widget), m_widgets.end());
}

void
Section::setActive(bool active)
{
   m_sectionActive = active;
   setVisible(active);

   for (auto& widget : m_widgets)
   {
      widget->setVisible(active);
   }
}

void
Section::UpdateWidgets(bool visible)
{
   for (auto& widget : m_widgets)
   {
      widget->setVisible(visible);
   }

   m_sectionActive = visible;
}