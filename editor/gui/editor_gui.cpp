#include "editor_gui.hpp"
#include "animatable.hpp"
#include "editor.hpp"
#include "game_object.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/types.hpp"
#include "renderer/vulkan_common.hpp"
#include "renderer/renderer.hpp"
#include "utils/file_manager.hpp"

#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cstdint>

namespace looper {

static ImGuiKey
ImGui_ImplGlfw_KeyToImGuiKey(int key)
{
   switch (key)
   {
      case GLFW_KEY_TAB:
         return ImGuiKey_Tab;
      case GLFW_KEY_LEFT:
         return ImGuiKey_LeftArrow;
      case GLFW_KEY_RIGHT:
         return ImGuiKey_RightArrow;
      case GLFW_KEY_UP:
         return ImGuiKey_UpArrow;
      case GLFW_KEY_DOWN:
         return ImGuiKey_DownArrow;
      case GLFW_KEY_PAGE_UP:
         return ImGuiKey_PageUp;
      case GLFW_KEY_PAGE_DOWN:
         return ImGuiKey_PageDown;
      case GLFW_KEY_HOME:
         return ImGuiKey_Home;
      case GLFW_KEY_END:
         return ImGuiKey_End;
      case GLFW_KEY_INSERT:
         return ImGuiKey_Insert;
      case GLFW_KEY_DELETE:
         return ImGuiKey_Delete;
      case GLFW_KEY_BACKSPACE:
         return ImGuiKey_Backspace;
      case GLFW_KEY_SPACE:
         return ImGuiKey_Space;
      case GLFW_KEY_ENTER:
         return ImGuiKey_Enter;
      case GLFW_KEY_ESCAPE:
         return ImGuiKey_Escape;
      case GLFW_KEY_APOSTROPHE:
         return ImGuiKey_Apostrophe;
      case GLFW_KEY_COMMA:
         return ImGuiKey_Comma;
      case GLFW_KEY_MINUS:
         return ImGuiKey_Minus;
      case GLFW_KEY_PERIOD:
         return ImGuiKey_Period;
      case GLFW_KEY_SLASH:
         return ImGuiKey_Slash;
      case GLFW_KEY_SEMICOLON:
         return ImGuiKey_Semicolon;
      case GLFW_KEY_EQUAL:
         return ImGuiKey_Equal;
      case GLFW_KEY_LEFT_BRACKET:
         return ImGuiKey_LeftBracket;
      case GLFW_KEY_BACKSLASH:
         return ImGuiKey_Backslash;
      case GLFW_KEY_RIGHT_BRACKET:
         return ImGuiKey_RightBracket;
      case GLFW_KEY_GRAVE_ACCENT:
         return ImGuiKey_GraveAccent;
      case GLFW_KEY_CAPS_LOCK:
         return ImGuiKey_CapsLock;
      case GLFW_KEY_SCROLL_LOCK:
         return ImGuiKey_ScrollLock;
      case GLFW_KEY_NUM_LOCK:
         return ImGuiKey_NumLock;
      case GLFW_KEY_PRINT_SCREEN:
         return ImGuiKey_PrintScreen;
      case GLFW_KEY_PAUSE:
         return ImGuiKey_Pause;
      case GLFW_KEY_KP_0:
         return ImGuiKey_Keypad0;
      case GLFW_KEY_KP_1:
         return ImGuiKey_Keypad1;
      case GLFW_KEY_KP_2:
         return ImGuiKey_Keypad2;
      case GLFW_KEY_KP_3:
         return ImGuiKey_Keypad3;
      case GLFW_KEY_KP_4:
         return ImGuiKey_Keypad4;
      case GLFW_KEY_KP_5:
         return ImGuiKey_Keypad5;
      case GLFW_KEY_KP_6:
         return ImGuiKey_Keypad6;
      case GLFW_KEY_KP_7:
         return ImGuiKey_Keypad7;
      case GLFW_KEY_KP_8:
         return ImGuiKey_Keypad8;
      case GLFW_KEY_KP_9:
         return ImGuiKey_Keypad9;
      case GLFW_KEY_KP_DECIMAL:
         return ImGuiKey_KeypadDecimal;
      case GLFW_KEY_KP_DIVIDE:
         return ImGuiKey_KeypadDivide;
      case GLFW_KEY_KP_MULTIPLY:
         return ImGuiKey_KeypadMultiply;
      case GLFW_KEY_KP_SUBTRACT:
         return ImGuiKey_KeypadSubtract;
      case GLFW_KEY_KP_ADD:
         return ImGuiKey_KeypadAdd;
      case GLFW_KEY_KP_ENTER:
         return ImGuiKey_KeypadEnter;
      case GLFW_KEY_KP_EQUAL:
         return ImGuiKey_KeypadEqual;
      case GLFW_KEY_LEFT_SHIFT:
         return ImGuiKey_LeftShift;
      case GLFW_KEY_LEFT_CONTROL:
         return ImGuiKey_LeftCtrl;
      case GLFW_KEY_LEFT_ALT:
         return ImGuiKey_LeftAlt;
      case GLFW_KEY_LEFT_SUPER:
         return ImGuiKey_LeftSuper;
      case GLFW_KEY_RIGHT_SHIFT:
         return ImGuiKey_RightShift;
      case GLFW_KEY_RIGHT_CONTROL:
         return ImGuiKey_RightCtrl;
      case GLFW_KEY_RIGHT_ALT:
         return ImGuiKey_RightAlt;
      case GLFW_KEY_RIGHT_SUPER:
         return ImGuiKey_RightSuper;
      case GLFW_KEY_MENU:
         return ImGuiKey_Menu;
      case GLFW_KEY_0:
         return ImGuiKey_0;
      case GLFW_KEY_1:
         return ImGuiKey_1;
      case GLFW_KEY_2:
         return ImGuiKey_2;
      case GLFW_KEY_3:
         return ImGuiKey_3;
      case GLFW_KEY_4:
         return ImGuiKey_4;
      case GLFW_KEY_5:
         return ImGuiKey_5;
      case GLFW_KEY_6:
         return ImGuiKey_6;
      case GLFW_KEY_7:
         return ImGuiKey_7;
      case GLFW_KEY_8:
         return ImGuiKey_8;
      case GLFW_KEY_9:
         return ImGuiKey_9;
      case GLFW_KEY_A:
         return ImGuiKey_A;
      case GLFW_KEY_B:
         return ImGuiKey_B;
      case GLFW_KEY_C:
         return ImGuiKey_C;
      case GLFW_KEY_D:
         return ImGuiKey_D;
      case GLFW_KEY_E:
         return ImGuiKey_E;
      case GLFW_KEY_F:
         return ImGuiKey_F;
      case GLFW_KEY_G:
         return ImGuiKey_G;
      case GLFW_KEY_H:
         return ImGuiKey_H;
      case GLFW_KEY_I:
         return ImGuiKey_I;
      case GLFW_KEY_J:
         return ImGuiKey_J;
      case GLFW_KEY_K:
         return ImGuiKey_K;
      case GLFW_KEY_L:
         return ImGuiKey_L;
      case GLFW_KEY_M:
         return ImGuiKey_M;
      case GLFW_KEY_N:
         return ImGuiKey_N;
      case GLFW_KEY_O:
         return ImGuiKey_O;
      case GLFW_KEY_P:
         return ImGuiKey_P;
      case GLFW_KEY_Q:
         return ImGuiKey_Q;
      case GLFW_KEY_R:
         return ImGuiKey_R;
      case GLFW_KEY_S:
         return ImGuiKey_S;
      case GLFW_KEY_T:
         return ImGuiKey_T;
      case GLFW_KEY_U:
         return ImGuiKey_U;
      case GLFW_KEY_V:
         return ImGuiKey_V;
      case GLFW_KEY_W:
         return ImGuiKey_W;
      case GLFW_KEY_X:
         return ImGuiKey_X;
      case GLFW_KEY_Y:
         return ImGuiKey_Y;
      case GLFW_KEY_Z:
         return ImGuiKey_Z;
      case GLFW_KEY_F1:
         return ImGuiKey_F1;
      case GLFW_KEY_F2:
         return ImGuiKey_F2;
      case GLFW_KEY_F3:
         return ImGuiKey_F3;
      case GLFW_KEY_F4:
         return ImGuiKey_F4;
      case GLFW_KEY_F5:
         return ImGuiKey_F5;
      case GLFW_KEY_F6:
         return ImGuiKey_F6;
      case GLFW_KEY_F7:
         return ImGuiKey_F7;
      case GLFW_KEY_F8:
         return ImGuiKey_F8;
      case GLFW_KEY_F9:
         return ImGuiKey_F9;
      case GLFW_KEY_F10:
         return ImGuiKey_F10;
      case GLFW_KEY_F11:
         return ImGuiKey_F11;
      case GLFW_KEY_F12:
         return ImGuiKey_F12;
      default:
         return ImGuiKey_None;
   }
}

static inline void
SetStyle()
{
   ImGuiStyle& style = ImGui::GetStyle();
   auto* colors = style.Colors; // NOLINT

   /// 0 = FLAT APPEARENCE
   /// 1 = MORE "3D" LOOK
   constexpr int is3D = 1;

   colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
   colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
   colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_WindowBg] = ImVec4(0.025f, 0.025f, 0.025f, 1.00f);
   colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
   colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
   colors[ImGuiCol_FrameBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.54f);
   colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
   colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
   colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
   colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
   colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
   colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
   colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
   colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
   colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
   colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
   colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
   colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
   colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
   colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
   colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
   colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
   colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
   colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
   colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
   colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
   colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
   colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
   colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
   colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
   colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
   colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
   colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
   colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
   colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
   colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
   colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
   colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
   colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

   style.PopupRounding = 3;

   style.WindowPadding = ImVec2(4, 4);
   style.FramePadding = ImVec2(6, 4);
   style.ItemSpacing = ImVec2(6, 2);

   style.ScrollbarSize = 18;

   style.WindowBorderSize = 1;
   style.ChildBorderSize = 1;
   style.PopupBorderSize = 1;
   style.FrameBorderSize = static_cast< float >(is3D);

   style.WindowRounding = 3;
   style.ChildRounding = 3;
   style.FrameRounding = 3;
   style.ScrollbarRounding = 2;
   style.GrabRounding = 3;

#ifdef IMGUI_HAS_DOCK
   style.TabBorderSize = is3D;
   style.TabRounding = 3;

   colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
   colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
   colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
   colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
   colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

   if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
   {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
   }
#endif
}

EditorGUI::EditorGUI(Editor& parent) : m_parent(parent)
{
}


void
EditorGUI::KeyCallback(const KeyEvent& event)
{
   auto* window = m_parent.GetWindow().GetWindowHandle();
   ImGuiIO& io = ImGui::GetIO();
   io.AddKeyEvent(ImGuiMod_Ctrl, (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                                    || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
   io.AddKeyEvent(ImGuiMod_Shift, (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                                     || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
   io.AddKeyEvent(ImGuiMod_Alt, (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
                                   || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
   io.AddKeyEvent(ImGuiMod_Super, (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS)
                                     || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));

   ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(event.m_key);
   io.AddKeyEvent(imgui_key, (event.m_action == GLFW_PRESS));
}

void
EditorGUI::CharCallback(const CharEvent& event)
{
   ImGuiIO& io = ImGui::GetIO();
   io.AddInputCharacter(event.m_key);
}

void
EditorGUI::MouseButtonCallback(const MouseButtonEvent& event)
{
   ImGuiIO& io = ImGui::GetIO();

   io.MouseDown[0] = (event.m_button == GLFW_MOUSE_BUTTON_1) && event.m_action;
   io.MouseDown[1] = (event.m_button == GLFW_MOUSE_BUTTON_2) && event.m_action;
}

void
EditorGUI::CursorPositionCallback(const CursorPositionEvent& event)
{
   ImGuiIO& io = ImGui::GetIO();
   io.MousePos = ImVec2(static_cast<float>(event.m_xPos), static_cast<float>(event.m_yPos));
}

void
EditorGUI::MouseScrollCallback(const MouseScrollEvent& /*event*/)
{
}


void
EditorGUI::Init()
{
   InputManager::RegisterForKeyInput(this);
   InputManager::RegisterForCharInput(this);
   InputManager::RegisterForMouseScrollInput(this);
   InputManager::RegisterForMouseButtonInput(this);
   InputManager::RegisterForMouseMovementInput(this);

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls NOLINT

   // Setup Dear ImGui style
   ImGui::StyleColorsDark();

   SetStyle();

   PrepareResources();

   auto& renderData = renderer::Data::renderData_.at(renderer::VulkanRenderer::GetCurrentlyBoundType());
   PreparePipeline(renderData.pipelineCache, renderData.renderPass);
}

void
EditorGUI::UpdateBuffers()
{
   ImDrawData* imDrawData = ImGui::GetDrawData();

   // Update buffers only if vertex or index count has been changed compared to current buffer size
   if (!imDrawData or !imDrawData->TotalVtxCount or !imDrawData->TotalIdxCount)
   {
      return;
   }

   // Note: Alignment is done inside buffer creation
   const VkDeviceSize vertexBufferSize =
      static_cast< uint32_t >(imDrawData->TotalVtxCount) * sizeof(ImDrawVert);
   const VkDeviceSize indexBufferSize =
      static_cast< uint32_t >(imDrawData->TotalIdxCount) * sizeof(ImDrawIdx);

   const auto currentFrame = renderer::Data::currentFrame_;

   // Vertex buffer
   if ((m_vertexBuffer[currentFrame].m_buffer == VK_NULL_HANDLE)
       || (vertexCount_[currentFrame] != imDrawData->TotalVtxCount))
   {
      m_vertexBuffer[currentFrame].Unmap();
      m_vertexBuffer[currentFrame].Destroy();

      m_vertexBuffer[currentFrame] = renderer::Buffer::CreateBuffer(
         vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      vertexCount_[currentFrame] = imDrawData->TotalVtxCount;
      m_vertexBuffer[currentFrame].Map();
   }

   // Index buffer
   if ((m_indexBuffer[currentFrame].m_buffer == VK_NULL_HANDLE)
       || (indexCount_[currentFrame] < imDrawData->TotalIdxCount))
   {
      m_indexBuffer[currentFrame].Unmap();
      m_indexBuffer[currentFrame].Destroy();

      m_indexBuffer[currentFrame] = renderer::Buffer::CreateBuffer(
         indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      indexCount_[currentFrame] = imDrawData->TotalIdxCount;
      m_indexBuffer[currentFrame].Map();
    }

   // Upload data
   auto* vtxDst = static_cast< ImDrawVert* >(m_vertexBuffer[currentFrame].m_mappedMemory);
   auto* idxDst = static_cast< ImDrawIdx* >(m_indexBuffer[currentFrame].m_mappedMemory);

   for (int n = 0; n < imDrawData->CmdListsCount; n++)
   {
      const ImDrawList* cmd_list = imDrawData->CmdLists[n];
      memcpy(vtxDst, cmd_list->VtxBuffer.Data,
             static_cast< size_t >(cmd_list->VtxBuffer.Size) * sizeof(ImDrawVert));
      memcpy(idxDst, cmd_list->IdxBuffer.Data,
             static_cast< size_t >(cmd_list->IdxBuffer.Size) * sizeof(ImDrawIdx));
      vtxDst += cmd_list->VtxBuffer.Size;
      idxDst += cmd_list->IdxBuffer.Size;
   }

   // Flush to make writes visible to GPU
   m_vertexBuffer[currentFrame].Flush();
   m_indexBuffer[currentFrame].Flush();
}

void
EditorGUI::Render(VkCommandBuffer commandBuffer)
{
   ImDrawData* imDrawData = ImGui::GetDrawData();
   int32_t vertexOffset = 0;
   uint32_t indexOffset = 0;

   if (!imDrawData or !imDrawData->CmdListsCount)
   {
      return;
   }

   UpdateBuffers();

   const ImGuiIO& io = ImGui::GetIO();
   const auto currentFrame = renderer::Data::currentFrame_;

   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                           &m_descriptorSet, 0, nullptr);

   m_pushConstant.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
   m_pushConstant.translate = glm::vec2(-1.0f);
   vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                      sizeof(PushConstBlock), &m_pushConstant);

   std::array< VkDeviceSize, 1 > offsets = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer[currentFrame].m_buffer,
                          offsets.data());
   vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer[currentFrame].m_buffer, 0,
                        VK_INDEX_TYPE_UINT16);

   for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
   {
      const ImDrawList* cmd_list = imDrawData->CmdLists[i];
      for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
      {
         const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
         VkRect2D scissorRect;
         scissorRect.offset.x = static_cast< int32_t >(glm::max(pcmd->ClipRect.x, 0.0f));
         scissorRect.offset.y = static_cast< int32_t >(glm::max(pcmd->ClipRect.y, 0.0f));
         scissorRect.extent.width = static_cast< uint32_t >(pcmd->ClipRect.z - pcmd->ClipRect.x);
         scissorRect.extent.height = static_cast< uint32_t >(pcmd->ClipRect.w - pcmd->ClipRect.y);
         vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
         vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
         indexOffset += pcmd->ElemCount;
      }
      vertexOffset += cmd_list->VtxBuffer.Size;
   }
}

void
EditorGUI::PrepareResources()
{
   ImGuiIO& io = ImGui::GetIO();

   // Create font texture
   unsigned char* fontData = nullptr;
   int32_t texWidth = 0;
   int32_t texHeight = 0;

   const auto fontFilename = (FONTS_DIR / "Roboto-Medium.ttf").string();

   io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), 16.0f);
   io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

   std::tie(m_fontImage, m_fontMemory) = renderer::Texture::CreateImage(
      static_cast< uint32_t >(texWidth), static_cast< uint32_t >(texHeight), 1,
      VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   m_fontView = renderer::Texture::CreateImageView(m_fontImage, VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_IMAGE_ASPECT_COLOR_BIT, 1);

   renderer::Texture::TransitionImageLayout(m_fontImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

   renderer::Texture::CopyBufferToImage(m_fontImage, static_cast< uint32_t >(texWidth),
                                        static_cast< uint32_t >(texHeight), fontData);

   renderer::Texture::TransitionImageLayout(m_fontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

   // Font texture Sampler
   m_sampler = renderer::Texture::CreateSampler();

   // Descriptor pool
   VkDescriptorPoolSize descriptorPoolSize{};
   descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorPoolSize.descriptorCount = 1;
   std::vector< VkDescriptorPoolSize > poolSizes = {descriptorPoolSize};

   VkDescriptorPoolCreateInfo descriptorPoolInfo{};
   descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   descriptorPoolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   descriptorPoolInfo.pPoolSizes = poolSizes.data();
   descriptorPoolInfo.maxSets = 2;

   renderer::vk_check_error(vkCreateDescriptorPool(renderer::Data::vk_device, &descriptorPoolInfo,
                                                   nullptr, &m_descriptorPool),
                            "");

   // Descriptor set layout
   VkDescriptorSetLayoutBinding setLayoutBinding{};
   setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
   setLayoutBinding.binding = 0;
   setLayoutBinding.descriptorCount = 1;

   std::vector< VkDescriptorSetLayoutBinding > setLayoutBindings = {setLayoutBinding};

   VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
   descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
   descriptorSetLayoutCreateInfo.bindingCount = static_cast< uint32_t >(setLayoutBindings.size());


   renderer::vk_check_error(vkCreateDescriptorSetLayout(renderer::Data::vk_device,
                                                        &descriptorSetLayoutCreateInfo, nullptr,
                                                        &m_descriptorSetLayout),
                            "");

   // Descriptor set
   VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
   descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
   descriptorSetAllocateInfo.pSetLayouts = &m_descriptorSetLayout;
   descriptorSetAllocateInfo.descriptorSetCount = 1;


   renderer::vk_check_error(vkAllocateDescriptorSets(renderer::Data::vk_device,
                                                     &descriptorSetAllocateInfo, &m_descriptorSet),
                            "");

   VkDescriptorImageInfo descriptorImageInfo{};
   descriptorImageInfo.sampler = m_sampler;
   descriptorImageInfo.imageView = m_fontView;
   descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

   VkWriteDescriptorSet writeDescriptorSet{};
   writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSet.dstSet = m_descriptorSet;
   writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   writeDescriptorSet.dstBinding = 0;
   writeDescriptorSet.pImageInfo = &descriptorImageInfo;
   writeDescriptorSet.descriptorCount = 1;

   std::vector< VkWriteDescriptorSet > writeDescriptorSets = {writeDescriptorSet};
   vkUpdateDescriptorSets(renderer::Data::vk_device,
                          static_cast< uint32_t >(writeDescriptorSets.size()),
                          writeDescriptorSets.data(), 0, nullptr);


   m_vertexBuffer.resize(renderer::Data::MAX_FRAMES_IN_FLIGHT);
   m_indexBuffer.resize(renderer::Data::MAX_FRAMES_IN_FLIGHT);
   vertexCount_.resize(renderer::Data::MAX_FRAMES_IN_FLIGHT);
   indexCount_.resize(renderer::Data::MAX_FRAMES_IN_FLIGHT);
}

void
EditorGUI::PreparePipeline(VkPipelineCache pipelineCache, VkRenderPass renderPass)
{
   // Pipeline layout
   // Push constants for UI rendering parameters
   VkPushConstantRange pushConstantRange{};
   pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
   pushConstantRange.offset = 0;
   pushConstantRange.size = sizeof(PushConstBlock);

   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
   pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutCreateInfo.setLayoutCount = 1;
   pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;

   pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
   pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
   renderer::vk_check_error(vkCreatePipelineLayout(renderer::Data::vk_device,
                                                   &pipelineLayoutCreateInfo, nullptr,
                                                   &m_pipelineLayout),
                            "");

   // Setup graphics pipeline for UI rendering
   VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
   pipelineInputAssemblyStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   pipelineInputAssemblyStateCreateInfo.flags = 0;
   pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

   VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
   pipelineRasterizationStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
   pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
   pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   pipelineRasterizationStateCreateInfo.flags = 0;
   pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
   pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;


   // Enable blending
   VkPipelineColorBlendAttachmentState blendAttachmentState{};
   blendAttachmentState.blendEnable = VK_TRUE;
   blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                         | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
   blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
   blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
   blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

   VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
   pipelineColorBlendStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   pipelineColorBlendStateCreateInfo.attachmentCount = 1;
   pipelineColorBlendStateCreateInfo.pAttachments = &blendAttachmentState;

   VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
   pipelineDepthStencilStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
   pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
   pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
   pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

   VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
   pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   pipelineViewportStateCreateInfo.viewportCount = 1;
   pipelineViewportStateCreateInfo.scissorCount = 1;
   pipelineViewportStateCreateInfo.flags = 0;

   VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
   pipelineMultisampleStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   pipelineMultisampleStateCreateInfo.rasterizationSamples = renderer::Data::msaaSamples;
   pipelineMultisampleStateCreateInfo.flags = 0;


   std::vector< VkDynamicState > dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                                        VK_DYNAMIC_STATE_SCISSOR};

   VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
   pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
   pipelineDynamicStateCreateInfo.dynamicStateCount =
      static_cast< uint32_t >(dynamicStateEnables.size());
   pipelineDynamicStateCreateInfo.flags = 0;

   auto [vertexInfo, fragmentInfo] =
      renderer::VulkanShader::CreateShader(renderer::Data::vk_device, "ui.vert.spv", "ui.frag.spv");
   std::array< VkPipelineShaderStageCreateInfo, 2 > shaderStages = {vertexInfo.shaderInfo,
                                                                    fragmentInfo.shaderInfo};

   VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
   pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineCreateInfo.layout = m_pipelineLayout;
   pipelineCreateInfo.renderPass = renderPass;
   pipelineCreateInfo.flags = 0;
   pipelineCreateInfo.basePipelineIndex = -1;
   pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
   pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
   pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
   pipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
   pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
   pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
   pipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
   pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
   pipelineCreateInfo.stageCount = 2;
   pipelineCreateInfo.pStages = shaderStages.data();
   pipelineCreateInfo.subpass = m_subpass;

   // Vertex bindings an attributes based on ImGui vertex definition
   VkVertexInputBindingDescription vInputBindDescription{};
   vInputBindDescription.binding = 0;
   vInputBindDescription.stride = sizeof(ImDrawVert);
   vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   std::vector< VkVertexInputBindingDescription > vertexInputBindings = {vInputBindDescription};

   VkVertexInputAttributeDescription pos{};
   pos.location = 0;
   pos.binding = 0;
   pos.format = VK_FORMAT_R32G32_SFLOAT;
   pos.offset = offsetof(ImDrawVert, pos);

   VkVertexInputAttributeDescription uv{};
   uv.location = 1;
   uv.binding = 0;
   uv.format = VK_FORMAT_R32G32_SFLOAT;
   uv.offset = offsetof(ImDrawVert, uv);

   VkVertexInputAttributeDescription color{};
   color.location = 2;
   color.binding = 0;
   color.format = VK_FORMAT_R8G8B8A8_UNORM;
   color.offset = offsetof(ImDrawVert, col);


   std::vector< VkVertexInputAttributeDescription > vertexInputAttributes = {
      pos,   // Location 0: Position
      uv,    // Location 1: UV
      color, // Location 2: Color
   };

   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
   pipelineVertexInputStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

   pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount =
      static_cast< uint32_t >(vertexInputBindings.size());
   pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindings.data();
   pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >(vertexInputAttributes.size());
   pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

   pipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

   renderer::vk_check_error(vkCreateGraphicsPipelines(renderer::Data::vk_device, pipelineCache, 1,
                                                      &pipelineCreateInfo, nullptr, &m_pipeline),
                            "");
}

void
EditorGUI::Shutdown()
{
   ImGui::DestroyContext();
}

bool
EditorGUI::IsBlockingEvents()
{
   const ImGuiIO& io = ImGui::GetIO();
   return io.WantCaptureMouse || io.WantTextInput;
}

void
EditorGUI::RenderCreateNewLevelWindow()
{
   const auto halfSize = m_windowSize / 2.0f;
   static glm::ivec2 size = {1024, 1024};
   static std::string name = "DummyLevelName";

   ImGui::SetNextWindowPos({halfSize.x, halfSize.y});
   ImGui::SetNextWindowSize({320, 120});
   ImGui::Begin("Create New");
   ImGui::InputInt2("Size", &size.x);
   ImGui::InputText("Name", name.data(), name.length());
   if (ImGui::Button("Create", {150, 35}))
   {
      m_parent.CreateLevel(name, size);
      m_createPushed = false;
   }
   ImGui::SameLine();
   if (ImGui::Button("Cancel", {150, 35}))
   {
      m_createPushed = false;
   }

   ImGui::End();
}

void
EditorGUI::RenderMainPanel()
{
   ImGui::SetNextWindowPos({0, 0});
   ImGui::SetNextWindowSize(ImVec2(m_windowWidth, m_toolsWindowHeight));
   ImGui::Begin("Tools");
   ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.45f, 0.0f, 0.2f, 0.8f});
   ImGui::BeginDisabled(m_currentLevel == nullptr);

   if (ImGui::Button("Play"))
   {
      m_parent.PlayLevel();
   }

   ImGui::PopStyleColor(1);
   ImGui::SameLine();
   if (ImGui::Button("Save"))
   {
      auto levelName = FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, true);
      if (!levelName.empty())
      {
         m_parent.SaveLevel(levelName);
      }
   }
   ImGui::EndDisabled();

   ImGui::SameLine();
   if (ImGui::Button("Load"))
   {
      auto levelName = FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, false);
      if (!levelName.empty())
      {
         m_parent.LoadLevel(levelName);
      }
   }
   ImGui::SameLine();
   if (ImGui::Button("Create") or m_createPushed)
   {
      m_createPushed = true;
      RenderCreateNewLevelWindow();
   }
   ImGui::End();
}

void
EditorGUI::RenderLevelMenu() // NOLINT
{
   ImGui::SetNextWindowPos({0, m_toolsWindowHeight});
   ImGui::SetNextWindowSize(ImVec2(m_windowWidth, m_levelWindowHeight));
   ImGui::Begin("Level");

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("General"))
   {
      auto sprite_size = m_currentLevel->GetSprite().GetSize();
      if (ImGui::InputFloat2("Size", &sprite_size.x))
      {
         m_currentLevel->SetSize(sprite_size);
      }

      auto [drawGrid, gridSize] = m_parent.GetGridData();
      if (ImGui::InputInt("cell size", &gridSize) || ImGui::Checkbox("Draw grid", &drawGrid))
      {
         m_parent.SetGridData(drawGrid, gridSize);
      }
   }

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Pathfinder"))
   {
      static bool renderPathfinderNodes = m_parent.GetRenderNodes();
      if (ImGui::Checkbox("Render nodes", &renderPathfinderNodes))
      {
         m_parent.RenderNodes(renderPathfinderNodes);
      }
   }

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Shader"))
   {
   }

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Objects"))
   {
      const auto& gameObjects = m_currentLevel->GetObjects();

      const auto items = std::to_array< std::string >({"Enemy", "Player", "Object"});
      ImGui::SetNextItemWidth(m_windowWidth * 0.95f);

      // The second parameter is the label previewed before opening the combo.
      if (ImGui::BeginCombo("##combo", "Add"))
      {
         for (const auto& item : items)
         {
            if (ImGui::Selectable(item.c_str()))
            {
               m_parent.AddGameObject(Object::GetTypeFromString(item));
            }
         }
         ImGui::EndCombo();
      }

      ImGui::BeginChild("Loaded Objects", {0, 200}, true);

      for (const auto& object : gameObjects)
      {
         auto label = fmt::format("[{}] {} ({}, {})", object->GetTypeString().c_str(),
                                  object->GetName().c_str(), object->GetPosition().x,
                                  object->GetPosition().y);

         if (ImGui::Selectable(label.c_str()))
         {
            m_parent.GetCamera().SetCameraAtPosition(object->GetPosition());
            m_parent.HandleGameObjectSelected(object, true);
         }
      }

      ImGui::EndChild();
   }


   ImGui::End();

   ImGui::SetNextWindowPos({m_windowWidth, m_windowSize.y - m_debugWindowHeight});
   ImGui::SetNextWindowSize(ImVec2(m_debugWindowWidth, m_debugWindowHeight));
   ImGui::Begin("Debug");
   ImGui::Text("Render time %s", m_parent.GetRenderTime().ToString().c_str());
   const auto cameraPos = m_parent.GetCamera().GetPosition();
   ImGui::Text("Camera Position %f, %f", static_cast< double >(cameraPos.x),
               static_cast< double >(cameraPos.y));

   const auto cameraZoom = m_parent.GetCamera().GetZoomLevel();
   const auto cameraRotation = m_parent.GetCamera().GetRotation();
   ImGui::Text("Camera Zoom %f Camera Rotation %f", static_cast< double >(cameraZoom),
               static_cast< double >(cameraRotation));

   const auto cursorOpengGLPos = m_parent.ScreenToGlobal(InputManager::GetMousePos());
   // const auto cursorOpengGLPos = InputManager::GetMousePos();
   ImGui::Text("Cursor Position %f, %f", static_cast< double >(cursorOpengGLPos.x),
               static_cast< double >(cursorOpengGLPos.y));

   auto& pathfinder = m_parent.GetLevel().GetPathfinder();
   const auto nodeID = pathfinder.GetNodeIDFromPosition(cursorOpengGLPos);
   Node curNode{};

   if (nodeID != -1)
   {
      curNode = pathfinder.GetNodeFromID(nodeID);
   }

   ImGui::Text("Cursor on tile ID = %d Coords(%d, %d)", curNode.m_ID, curNode.m_xPos,
               curNode.m_yPos);

   ImGui::End();
}

void
EditorGUI::RenderGameObjectMenu() // NOLINT
{
   ImGui::SetNextWindowPos({m_windowSize.x - m_windowWidth, 0});
   ImGui::SetNextWindowSize(ImVec2(m_windowWidth, m_gameObjectWindowHeight));
   ImGui::Begin("Game Object");
   ImGui::SetNextItemOpen(true);

   if (ImGui::CollapsingHeader("General"))
   {
      auto name = m_currentlySelectedGameObject->GetName();
      const auto nameLength = 20;
      name.resize(nameLength);

      auto type = m_currentlySelectedGameObject->GetTypeString();

      ImGui::InputText("Type", type.data(), type.size(), ImGuiInputTextFlags_ReadOnly);

      if (ImGui::InputText("Name", name.data(), nameLength))
      {
         m_currentlySelectedGameObject->SetName(name);
      }

      auto collision = m_currentlySelectedGameObject->GetHasCollision();
      if (ImGui::Checkbox("Has Collision", &collision))
      {
         m_currentlySelectedGameObject->SetHasCollision(collision);
      }
   }

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Transform"))
   {
      auto objectPosition = m_currentlySelectedGameObject->GetPosition();
      auto sprite_size = m_currentlySelectedGameObject->GetSprite().GetSize();
      auto rotation = m_currentlySelectedGameObject->GetSprite().GetRotation(
         renderer::Sprite::RotationType::DEGREES);

      ImGui::InputFloat2("Position", &objectPosition.x);

      if (ImGui::SliderFloat2("Size", &sprite_size.x, 10, 1000))
      {
         m_currentlySelectedGameObject->SetSize(sprite_size);
      }

      if (ImGui::SliderFloat("Rotate", &rotation,
                             glm::degrees(renderer::Sprite::ROTATION_RANGE.first),
                             glm::degrees(renderer::Sprite::ROTATION_RANGE.second)))
      {
         m_currentlySelectedGameObject->Rotate(glm::radians(rotation));
      }
   }

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Shader"))
   {
      if (m_currentLevel)
      {
         auto& sprite = m_currentlySelectedGameObject->GetSprite();
         // TODO: fix it!
         // ImGui::Image(reinterpret_cast< void* >( // NOLINT
         //                 static_cast< size_t >(sprite.GetTexture().GetImage())),
         //              {150, 150});
         ImGui::InputText("FileName", sprite.GetTextureName().data(),
                          sprite.GetTextureName().size(), ImGuiInputTextFlags_ReadOnly);
         if (ImGui::Button("Change Texture"))
         {
            auto textureName = FileManager::FileDialog(
               IMAGES_DIR, {{"PNG texture", "png"}, {"JPEG texture", "jpg"}}, false);
            if (!textureName.empty())
            {
               sprite.SetTextureFromFile(textureName);
            }
         }
      }
   }

   if (m_currentlySelectedGameObject->GetType() == ObjectType::ENEMY)
   {
      const auto animatablePtr =
         std::dynamic_pointer_cast< Animatable >(m_currentlySelectedGameObject);

      ImGui::SetNextItemOpen(true);
      if (ImGui::CollapsingHeader("Animation"))
      {
         ImGui::Text("Type"); // NOLINT
         ImGui::SameLine();

         if (ImGui::RadioButton("Loop", animatablePtr->GetAnimationType()
                                           == Animatable::ANIMATION_TYPE::LOOP))
         {
            animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::LOOP);
         }

         ImGui::SameLine();
         if (ImGui::RadioButton("Reversal", animatablePtr->GetAnimationType()
                                               == Animatable::ANIMATION_TYPE::REVERSABLE))
         {
            animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::REVERSABLE);
         }

         bool animationVisible = animatablePtr->GetRenderAnimationSteps();
         if (ImGui::Checkbox("Animation points visible", &animationVisible))
         {
            m_parent.SetRenderAnimationPoints(animationVisible);
         }

         if (ImGui::Button("Animate"))
         {
            m_parent.ToggleAnimateObject();
         }


         static float timer = 0.0f;
         const auto animationDuration =
            static_cast< float >(Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count());
         if (m_parent.IsObjectAnimated())
         {
            timer += static_cast< float >(m_parent.GetDeltaTime().count());
            timer = glm::min(animationDuration, timer);
         }

         ImGui::SameLine();
         if (ImGui::SliderFloat("##", &timer, 0.0f, animationDuration, "%.3f ms"))
         {
            m_currentlySelectedGameObject->GetSprite().SetTranslateValue(
               animatablePtr->SetAnimation(Timer::milliseconds(static_cast< uint64_t >(timer))));
         }

         // static int selected = 0;
         auto animationPoints = animatablePtr->GetAnimationKeypoints();
         auto newNodePosition = m_currentlySelectedGameObject->GetPosition();
         ImGui::BeginChild("Animation Points", {0, 100}, true);
         for (uint32_t i = 0; i < animationPoints.size(); ++i)
         {
            const auto& node = animationPoints[i];
            auto label = fmt::format("[{}] Pos({:.{}f},{:.{}f}) Time={}s", i, node.m_end.x, 1,
                                     node.m_end.y, 1, node.m_timeDuration.count());
            if (ImGui::Selectable(label.c_str()))
            {
               m_parent.GetCamera().SetCameraAtPosition(node.m_end);
               m_parent.HandleObjectSelected(node.GetID(), true);
               m_parent.SetRenderAnimationPoints(true);
            }

            newNodePosition = node.m_end;
         }

         if (ImGui::Button("New"))
         {
            m_parent.GetCamera().SetCameraAtPosition(newNodePosition);
            m_parent.AddObject(ObjectType::ANIMATION_POINT);
            m_parent.SetRenderAnimationPoints(true);
         }
         ImGui::EndChild();
      }
   }

   ImGui::End();
}

void
EditorGUI::UpdateUI()
{
   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2(m_parent.GetWindowSize().x, m_parent.GetWindowSize().y);

   ImGui::NewFrame();

   m_windowSize = m_parent.GetWindowSize();

   m_windowWidth = m_windowSize.x / 7;
   m_toolsWindowHeight = 60;
   m_levelWindowHeight = m_windowSize.y - m_toolsWindowHeight;
   m_gameObjectWindowHeight = m_windowSize.y;
   m_debugWindowWidth = m_windowSize.x - 2 * m_windowWidth;
   m_debugWindowHeight = 100;

   RenderMainPanel();

   if (m_currentLevel)
   {
      RenderLevelMenu();
   }

   if (m_currentlySelectedGameObject)
   {
      RenderGameObjectMenu();
   }

   ImGui::Render();
}

void
EditorGUI::GameObjectSelected(const std::shared_ptr< GameObject >& selectedGameObject)
{
   m_currentlySelectedGameObject = selectedGameObject;
}

void
EditorGUI::GameObjectUnselected()
{
   m_currentlySelectedGameObject = nullptr;
}

void
EditorGUI::EditorObjectSelected(const std::shared_ptr< EditorObject >& /*object*/)
{
   // m_currentlySelectedEditorObject = object;
}

void
EditorGUI::EditorObjectUnselected()
{
   // m_currentlySelectedEditorObject = nullptr;
}

void
EditorGUI::LevelLoaded(const std::shared_ptr< Level >& loadedLevel)
{
   m_currentLevel = loadedLevel;
}

void
EditorGUI::ObjectUpdated(Object::ID /*ID*/)
{
}

void
EditorGUI::ObjectDeleted(Object::ID /*ID*/)
{
}

} // namespace looper
