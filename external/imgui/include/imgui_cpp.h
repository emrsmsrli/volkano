#ifndef IMGUICPP_IMGUI_CPP_H
#define IMGUICPP_IMGUI_CPP_H

#include <array>
#include <string>
#include <string_view>

#include <fmt/format.h>

#include "imgui.h"
#include "imgui_internal.h"

#define IM_WRAPPER_STRUCT(ClassName)                        \
    ClassName(const ClassName&) = delete;                   \
    ClassName(ClassName&&) noexcept = delete;               \
    ClassName& operator=(const ClassName&) = delete;        \
    ClassName& operator=(ClassName&&) noexcept = delete;    \

namespace ImGuiCpp {

struct [[nodiscard]] Window {
    bool shown = false;

    explicit Window(const std::string_view name, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
    {
        shown = ImGui::Begin(name.data(), nullptr, flags);
    }

    ~Window() { ImGui::End(); }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(Window);
};

struct [[nodiscard]] ClosableWindow {
    bool shown = false;
    bool is_open = true;

    explicit ClosableWindow(const std::string_view name, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
    {
        shown = ImGui::Begin(name.data(), &is_open, flags);
    }

    ~ClosableWindow() { ImGui::End(); }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(ClosableWindow)
};

struct [[nodiscard]] WindowChild {
    bool shown = false;

    explicit WindowChild(const std::string_view id_view, const ImVec2& size = ImVec2(0, 0), const bool border = false, const ImGuiWindowFlags flags = 0)
    {
        shown = ImGui::BeginChild(id_view.data(), size, border, flags);
    }

    explicit WindowChild(const ImGuiID id, const ImVec2& size = ImVec2(0, 0), const bool border = false, const ImGuiWindowFlags flags = 0)
    {
        shown = ImGui::BeginChild(id, size, border, flags);
    }

    ~WindowChild() { ImGui::EndChild(); }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(WindowChild);
};

struct [[nodiscard]] Group {
    template<typename F>
    explicit Group(F&& f) { ImGui::BeginGroup(); f(); }
    ~Group() { ImGui::EndGroup(); }

    IM_WRAPPER_STRUCT(Group);
};

struct [[nodiscard]] MainMenuBar {
    bool shown = false;

    explicit MainMenuBar() { shown = ImGui::BeginMainMenuBar(); }
    ~MainMenuBar() { if(shown) { ImGui::EndMainMenuBar(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(MainMenuBar);
};

struct [[nodiscard]] MenuBar {
    bool shown = false;

    explicit MenuBar() { shown = ImGui::BeginMenuBar(); }
    ~MenuBar() { if(shown) { ImGui::EndMenuBar(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(MenuBar);
};

struct [[nodiscard]] Menu {
    bool shown = false;

    explicit Menu(const std::string_view label, bool enabled = true) { shown = ImGui::BeginMenu(label.data(), enabled); }
    ~Menu() { if(shown) { ImGui::EndMenu(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(Menu);
};

struct [[nodiscard]] Tooltip {
    template<typename F>
    explicit Tooltip(F&& f) { ImGui::BeginTooltip(); f(); }
    ~Tooltip() { ImGui::EndTooltip(); }

    IM_WRAPPER_STRUCT(Tooltip);
};

struct [[nodiscard]] Combo {
    bool shown = false;

    explicit Combo(const std::string_view label, const std::string_view preview, const ImGuiComboFlags flags = ImGuiComboFlags_None)
    {
        shown = ImGui::BeginCombo(label.data(), preview.data(), flags);
    }

    ~Combo() { if(shown) { ImGui::EndCombo(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(Combo);
};

struct [[nodiscard]] TreeNode {
    bool shown = false;

    explicit TreeNode(const std::string_view label, const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None)
    {
        shown = ImGui::TreeNodeEx(label.data(), flags);
    }

    ~TreeNode() { if(shown) { ImGui::TreePop(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(TreeNode);
};

struct [[nodiscard]] TreeIndent {
    bool shown = false;

    explicit TreeIndent(const std::string_view id)
    {
        shown = ImGui::TreePush(id.data());
    }

    ~TreeIndent() { if(shown) { ImGui::TreePop(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(TreeIndent);
};

struct [[nodiscard]] ListBox {
    bool shown = false;

    explicit ListBox(const std::string_view label, const ImVec2& size = ImVec2(0, 0))
    {
        shown = ImGui::BeginListBox(label.data(), size);
    }

    ~ListBox() { if(shown) { ImGui::EndListBox(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(ListBox);
};

struct [[nodiscard]] Popup {
    bool shown = false;

    explicit Popup(const std::string_view id, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
    {
        shown = ImGui::BeginPopup(id.data(), flags);
    }

    ~Popup() { if(shown) { ImGui::EndPopup(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(Popup);
};

struct [[nodiscard]] PopupModal {
    bool shown = false;

    explicit PopupModal(const std::string_view name, bool* p_open = nullptr, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
    {
        shown = ImGui::BeginPopupModal(name.data(), p_open, flags);
    }

    ~PopupModal() { if(shown) { ImGui::EndPopup(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(PopupModal);
};

struct [[nodiscard]] Table {
    bool shown = false;

    explicit Table(const std::string_view id, const int n_column, const ImGuiTableFlags flags = ImGuiTableFlags_None, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), const float inner_width = 0.0f)
    {
        shown = ImGui::BeginTable(id.data(), n_column, flags, outer_size, inner_width);
    }

    ~Table() { if(shown) { ImGui::EndTable(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(Table);
};

struct [[nodiscard]] TabBar {
    bool shown = false;

    explicit TabBar(const std::string_view id, const ImGuiTabBarFlags flags = ImGuiTabBarFlags_None)
    {
        shown = ImGui::BeginTabBar(id.data(), flags);
    }

    ~TabBar() { if(shown) { ImGui::EndTabBar(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(TabBar);
};

struct [[nodiscard]] TabItem {
    bool shown = false;

    explicit TabItem(const std::string_view label, bool* p_open = nullptr, const ImGuiTabItemFlags flags = ImGuiTabItemFlags_None)
    {
        shown = ImGui::BeginTabItem(label.data(), p_open, flags);
    }

    ~TabItem() { if(shown) { ImGui::EndTabItem(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(TabItem);
};

struct [[nodiscard]] DragDropSource {
    bool shown = false;

    explicit DragDropSource(const ImGuiDragDropFlags flags = ImGuiDragDropFlags_None)
    {
        shown = ImGui::BeginDragDropSource(flags);
    }

    ~DragDropSource() { if(shown) { ImGui::EndDragDropSource(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(DragDropSource);
};

struct [[nodiscard]] DragDropTarget {
    bool shown = false;

    explicit DragDropTarget(const ImGuiDragDropFlags flags = ImGuiDragDropFlags_None)
    {
        shown = ImGui::BeginDragDropTarget(flags);
    }

    ~DragDropTarget() { if(shown) { ImGui::EndDragDropTarget(); } }
    operator bool() const { return shown; }

    IM_WRAPPER_STRUCT(DragDropTarget);
};

struct [[nodiscard]] DisabledGroup {
    template<typename F>
    explicit DisabledGroup(F&& f, bool disabled = true) { ImGui::BeginDisabled(disabled); f(); }
    ~DisabledGroup() { ImGui::EndDisabled(); }

    IM_WRAPPER_STRUCT(DisabledGroup);
};

struct [[nodiscard]] ClipRect {
    explicit ClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
    {
        ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    }

    ~ClipRect() { ImGui::PopClipRect(); }

    IM_WRAPPER_STRUCT(ClipRect);
};

void TextUnformatted(const std::string_view text)
{
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
}

template<typename Fmt, typename... Args>
void Text(Fmt&& fmt, Args&&... args)
{
    ImGui::TextUnformatted(fmt::format(std::forward<Fmt>(fmt), std::forward<Args>(args)...).c_str());
}

template<typename Fmt, typename... Args>
void TextColored(const ImVec4& col, Fmt&& fmt, Args&&... args)
{
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::TextUnformatted(fmt::format(std::forward<Fmt>(fmt), std::forward<Args>(args)...).c_str());
    ImGui::PopStyleColor();
}

template<typename Fmt, typename... Args>
void TextDisabled(Fmt&& fmt, Args&&... args)
{
    TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), std::forward<Fmt>(fmt), std::forward<Args>(args)...);
}

template<typename Fmt, typename... Args>
void TextWrapped(Fmt&& fmt, Args&&... args)
{
    ImGuiContext* c = ImGui::GetCurrentContext();
    const bool need_backup = c->CurrentWindow->DC.TextWrapPos < 0.0f;  // Keep existing wrap position if one is already set
    if(need_backup) {
        ImGui::PushTextWrapPos(0.0f);
    }
    Text(std::forward<Fmt>(fmt), std::forward<Args>(args)...);
    if(need_backup) {
        ImGui::PopTextWrapPos();
    }
}

bool Button(const std::string_view label, const ImVec2& size = ImVec2(0, 0))
{
    return ImGui::Button(label.data(), size);
}

bool SmallButton(const std::string_view label)
{
    return ImGui::SmallButton(label.data());
}

} // namespace ImGui

#undef IM_WRAPPER_STRUCT

#endif // IMGUICPP_IMGUI_CPP_H
