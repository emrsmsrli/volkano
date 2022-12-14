/*
 * Copyright (C) 2022 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#ifndef IMGUICPP_IMGUI_CPP_H
#define IMGUICPP_IMGUI_CPP_H

#include <string_view>

#include <fmt/core.h>

#include "imgui.h"
#include "imgui_internal.h"

namespace Im {

struct Context {
    ImGuiContext* ctx = nullptr;

    explicit Context(ImFontAtlas* atlas = nullptr) : ctx{ImGui::CreateContext(atlas)} {}
    ~Context() { ImGui::DestroyContext(ctx); }

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

    void set_current() const { ImGui::SetCurrentContext(ctx); }
};

namespace detail {

template<bool HasActive = true>
struct RaiiBase {
    bool active = true;

    RaiiBase(const RaiiBase&) = delete;
    RaiiBase(RaiiBase&&) noexcept = delete;
    RaiiBase& operator=(const RaiiBase&) = delete;
    RaiiBase& operator=(RaiiBase&&) noexcept = delete;

    template<typename Func>
    void operator<<(Func&& func) const noexcept
    {
        if constexpr (HasActive) {
            if (!active) {
                return;
            }
        }

        func();
    }

protected:
    // not constructible by itself
    RaiiBase() = default;
    explicit RaiiBase(const bool active) noexcept : active(active) {}
};

using RaiiNoActive = RaiiBase<false>;
using Raii = RaiiBase<true>;

} // namespace detail

/* -------WINDOWS ------ */

struct Window final : detail::Raii {
    explicit Window(const std::string_view name, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
        : Window(name, nullptr, flags) {}

    explicit Window(const std::string_view name, bool* p_open, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
        : detail::Raii(ImGui::Begin(name.data(), p_open, flags)) {}

    ~Window() { ImGui::End(); }
};

struct ClosableWindow final : detail::Raii {
    bool is_open = true;

    explicit ClosableWindow(const std::string_view name, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
        : detail::Raii(ImGui::Begin(name.data(), &is_open, flags)) {}

    ~ClosableWindow() { ImGui::End(); }
};

struct ChildWindow final : detail::Raii {
    explicit ChildWindow(const std::string_view id, const ImVec2& size = {0.f, 0.f}, const bool border = false, const ImGuiWindowFlags flags = 0)
        : detail::Raii(ImGui::BeginChild(id.data(), size, border, flags)) {}

    explicit ChildWindow(const ImGuiID id, const ImVec2& size = {0.f, 0.f}, const bool border = false, const ImGuiWindowFlags flags = 0)
        : detail::Raii(ImGui::BeginChild(id, size, border, flags)) {}

    ~ChildWindow() { ImGui::EndChild(); }
};

/* -------WIDGETS ------ */

struct MainMenuBar final : detail::Raii {
    MainMenuBar() : detail::Raii(ImGui::BeginMainMenuBar()) {}
    ~MainMenuBar() { if (active) { ImGui::EndMainMenuBar(); } }
};

struct MenuBar final : detail::Raii {
    MenuBar() : detail::Raii(ImGui::BeginMenuBar()) {}
    ~MenuBar() { if (active) { ImGui::EndMenuBar(); } }
};

struct Menu final : detail::Raii {
    explicit Menu(const std::string_view label, bool enabled = true)
        : detail::Raii(ImGui::BeginMenu(label.data(), enabled)) {}
    ~Menu() { if (active) { ImGui::EndMenu(); } }
};

struct Tooltip final : detail::RaiiNoActive {
    Tooltip() { ImGui::BeginTooltip(); }
    ~Tooltip() { ImGui::EndTooltip(); }
};

struct HoveredTooltip final : detail::Raii {
    explicit HoveredTooltip(ImGuiHoveredFlags flags = ImGuiHoveredFlags_None)
        : detail::Raii(ImGui::IsItemHovered(flags))
    {
        if (active) {
            ImGui::BeginTooltip();
        }
    }

    ~HoveredTooltip() { if (active) { ImGui::EndTooltip(); } }
};

struct Combo final : detail::Raii {
    Combo(const std::string_view label, const std::string_view preview, const ImGuiComboFlags flags = ImGuiComboFlags_None)
        : detail::Raii(ImGui::BeginCombo(label.data(), preview.data(), flags)) {}

    ~Combo() { if (active) { ImGui::EndCombo(); } }
};

struct TreeNode final : detail::Raii {
    explicit TreeNode(const std::string_view label, const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None)
        : detail::Raii(ImGui::TreeNodeEx(label.data(), flags)) {}

    ~TreeNode() { if (active) { ImGui::TreePop(); } }
};

struct TreeIndent final : detail::RaiiNoActive {
    explicit TreeIndent(const void* ptr_id = nullptr) { ImGui::TreePush(ptr_id); }
    explicit TreeIndent(const std::string_view id) { ImGui::TreePush(id.data()); }
    ~TreeIndent() { ImGui::TreePop(); }
};

struct ListBox final : detail::Raii {
    explicit ListBox(const std::string_view label, const ImVec2& size = {0.f, 0.f})
        : detail::Raii(ImGui::BeginListBox(label.data(), size)) {}

    ~ListBox() { if (active) { ImGui::EndListBox(); } }
};

struct Table final : detail::Raii {
    Table(const std::string_view id, const int n_columns, const ImGuiTableFlags flags = ImGuiTableFlags_None, const ImVec2& outer_size = {0.f, 0.f}, const float inner_width = 0.0f)
        : detail::Raii(ImGui::BeginTable(id.data(), n_columns, flags, outer_size, inner_width)) {}

    ~Table() { if (active) { ImGui::EndTable(); } }
};

struct TabBar final : detail::Raii {
    explicit TabBar(const std::string_view id, const ImGuiTabBarFlags flags = ImGuiTabBarFlags_None)
        : detail::Raii(ImGui::BeginTabBar(id.data(), flags)) {}

    ~TabBar() { if (active) { ImGui::EndTabBar(); } }
};

struct TabItem final : detail::Raii {
    explicit TabItem(const std::string_view label, const ImGuiTabItemFlags flags = ImGuiTabItemFlags_None)
        : TabItem(label, nullptr, flags) {}
    explicit TabItem(const std::string_view label, bool* p_open, const ImGuiTabItemFlags flags = ImGuiTabItemFlags_None)
        : detail::Raii(ImGui::BeginTabItem(label.data(), p_open, flags)) {}

    ~TabItem() { if (active) { ImGui::EndTabItem(); } }
};

/* -------POPUPS ------ */

struct Popup final : detail::Raii {
    explicit Popup(const std::string_view id, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
        : detail::Raii(ImGui::BeginPopup(id.data(), flags)) {}

    ~Popup() { if (active) { ImGui::EndPopup(); } }
};

struct PopupModal final : detail::Raii {
    explicit PopupModal(const std::string_view name, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
        : PopupModal(name, nullptr, flags) {}
    explicit PopupModal(const std::string_view name, bool* p_open, const ImGuiWindowFlags flags = ImGuiWindowFlags_None)
        : detail::Raii(ImGui::BeginPopupModal(name.data(), p_open, flags)) {}

    ~PopupModal() { if (active) { ImGui::EndPopup(); } }
};

struct PopupContext_Item final : detail::Raii {
    explicit PopupContext_Item(const std::string_view name, const ImGuiPopupFlags flags = ImGuiPopupFlags_MouseButtonRight)
        : detail::Raii(ImGui::BeginPopupContextItem(name.data(), flags)) {}

    ~PopupContext_Item() { if (active) { ImGui::EndPopup(); } }
};

struct PopupContext_Window final : detail::Raii {
    explicit PopupContext_Window(const std::string_view name, const ImGuiPopupFlags flags = ImGuiPopupFlags_MouseButtonRight)
        : detail::Raii(ImGui::BeginPopupContextWindow(name.data(), flags)) {}

    ~PopupContext_Window() { if (active) { ImGui::EndPopup(); } }
};

struct PopupContext_Void final : detail::Raii {
    explicit PopupContext_Void(const std::string_view name, const ImGuiPopupFlags flags = ImGuiPopupFlags_MouseButtonRight)
        : detail::Raii(ImGui::BeginPopupContextVoid(name.data(), flags)) {}

    ~PopupContext_Void() { if (active) { ImGui::EndPopup(); } }
};

/* -------DRAG & DROP------- */

struct [[nodiscard]] DragDropSource final : detail::Raii {
    explicit DragDropSource(const ImGuiDragDropFlags flags = ImGuiDragDropFlags_None)
        : detail::Raii(ImGui::BeginDragDropSource(flags)) {}

    ~DragDropSource() { if (active) { ImGui::EndDragDropSource(); } }

    template<typename T>
    const DragDropSource& submit(const std::string_view type, const T& data, const ImGuiCond condition = ImGuiCond_None) const noexcept
    {
        if (active) {
            ImGui::SetDragDropPayload(type.data(), &data, sizeof(T), condition);
        }
        return *this;
    }
};

struct [[nodiscard]] DragDropTarget final : detail::Raii {
    DragDropTarget() : detail::Raii(ImGui::BeginDragDropTarget()) {}
    ~DragDropTarget() { if (active) { ImGui::EndDragDropTarget(); } }

    template<typename T, typename Func>
    const DragDropTarget& accept(const std::string_view type, Func&& func) const noexcept
    {
        if (active) {
            if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload(type.data())) {
                func(static_cast<T*>(p->Data));
            }
        }
        return *this;
    }
};

/* -------SCOPES------- */

struct DisabledScope final : detail::RaiiNoActive {
    explicit DisabledScope(const bool disabled = true) { ImGui::BeginDisabled(disabled); }
    ~DisabledScope() { ImGui::EndDisabled(); }
};

struct ClipRect final : detail::RaiiNoActive {
    explicit ClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
    {
        ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    }

    ~ClipRect() { ImGui::PopClipRect(); }
};

struct FontScope final : detail::RaiiNoActive {
    explicit FontScope(ImFont* font) { ImGui::PushFont(font); }
    ~FontScope() { ImGui::PopFont(); }
};

struct StyleColorScope final : detail::RaiiNoActive {
    int count = 0;

    StyleColorScope(const ImGuiCol idx, const ImU32 col) { ImGui::PushStyleColor(idx, col); }
    StyleColorScope(const ImGuiCol idx, const ImVec4& col) { ImGui::PushStyleColor(idx, col); }
    ~StyleColorScope() { ImGui::PopStyleColor(count); }

    StyleColorScope operator+(const StyleColorScope& other) const noexcept
    {
        return StyleColorScope{count + other.count};
    }

private:
    explicit StyleColorScope(const int c) : count(c) {}
};

struct AllowKeyboardFocusScope final : detail::RaiiNoActive {
    explicit AllowKeyboardFocusScope(const bool allow_keyboard_focus) { ImGui::PushAllowKeyboardFocus(allow_keyboard_focus); }
    ~AllowKeyboardFocusScope() { ImGui::PopAllowKeyboardFocus(); }
};

struct ButtonRepeatScope final : detail::RaiiNoActive {
    explicit ButtonRepeatScope(const bool repeat) { ImGui::PushButtonRepeat(repeat); }
    ~ButtonRepeatScope() { ImGui::PopButtonRepeat(); }
};

struct ItemWidthScope final : detail::RaiiNoActive {
    explicit ItemWidthScope(const float item_width) { ImGui::PushItemWidth(item_width); }
    ~ItemWidthScope() { ImGui::PopItemWidth(); }
};

struct TextWrapPosScope final : detail::RaiiNoActive {
    explicit TextWrapPosScope(const float wrap_local_pos_x) { ImGui::PushTextWrapPos(wrap_local_pos_x); }
    ~TextWrapPosScope() { ImGui::PopTextWrapPos(); }
};

struct IDScope final : detail::RaiiNoActive {
    explicit IDScope(const char* str_id) { ImGui::PushID(str_id); }
    explicit IDScope(const char* str_id_begin, const char* str_id_end) { ImGui::PushID(str_id_begin, str_id_end); }
    explicit IDScope(const void* ptr_id) { ImGui::PushID(ptr_id); }
    explicit IDScope(const int id) { ImGui::PushID(id); }

    ~IDScope() { ImGui::PopID(); }
};

/* -------LAYOUT ------ */

struct Indent final : detail::RaiiNoActive {
    float indent = 0.f;

    explicit Indent(const float indent_w = 0.0f) : indent(indent_w) { ImGui::Indent(indent_w); }
    ~Indent() { ImGui::Unindent(indent); }
};

struct Group final : detail::RaiiNoActive {
    Group() { ImGui::BeginGroup(); }
    ~Group() { ImGui::EndGroup(); }
};

/* -------FUNCTIONS ------ */

template<typename Fmt, typename... Args>
std::string_view FormatText(Fmt&& fmt, Args&&... args)
{
    ImGuiContext* c = ImGui::GetCurrentContext();
    const auto end = fmt::format_to(c->TempBuffer, std::forward<Fmt>(fmt), std::forward<Args>(args)...);
    return std::string_view{c->TempBuffer, static_cast<size_t>(std::distance(c->TempBuffer, end))};
}

void TextUnformatted(const std::string_view text)
{
    ImGui::TextUnformatted(text.data(), text.data() + text.size());
}

template<typename Fmt, typename... Args>
void Text(Fmt&& fmt, Args&&... args)
{
    const std::string_view formatted = FormatText(std::forward<Fmt>(fmt), std::forward<Args>(args)...);
    TextUnformatted(formatted);
}

template<typename Fmt, typename... Args>
void TextColored(const ImVec4& col, Fmt&& fmt, Args&&... args)
{
    StyleColorScope color{ImGuiCol_Text, col};
    Text(std::forward<Fmt>(fmt), std::forward<Args>(args)...);
}

void TextColoredUnformatted(const ImVec4& col, const std::string_view text)
{
    StyleColorScope color{ImGuiCol_Text, col};
    TextUnformatted(text);
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

bool InvisibleButton(const std::string_view id, const ImVec2& size, const ImGuiButtonFlags flags = ImGuiButtonFlags_None)
{
    return ImGui::InvisibleButton(id.data(), size, flags);
}

bool ArrowButton(const std::string_view id, const ImGuiDir dir)
{
    return ImGui::ArrowButton(id.data(), dir);
}

bool Checkbox(const std::string_view label, bool& enabled)
{
    return ImGui::Checkbox(label.data(), &enabled);
}

template<typename T>
bool CheckboxFlags(const std::string_view label, T& flags, const T flag_to_set)
{
    static_assert(std::is_convertible_v<T, int>);
    return ImGui::Checkbox(label.data(), &flags, flag_to_set);
}

template<typename Buffer>
bool InputText(const std::string_view label, Buffer& buffer, const ImGuiInputTextFlags flags = ImGuiInputTextFlags_None, const ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
{
    static_assert(std::is_same_v<char, typename Buffer::value_type> || std::is_same_v<char[], Buffer>);
    return ImGui::InputText(label.data(), buffer.data(), buffer.size(), flags, callback, user_data);
}

template<typename Buffer>
bool InputTextMultiline(const std::string_view label, Buffer& buffer, ImVec2 size = {0.f, 0.f},
  const ImGuiInputTextFlags flags = ImGuiInputTextFlags_None, const ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
{
    static_assert(std::is_same_v<char, typename Buffer::value_type> || std::is_same_v<char[], Buffer>);
    return ImGui::InputTextMultiline(label.data(), buffer.data(), buffer.size(), size, flags, callback, user_data);
}

} // namespace Im

#endif // IMGUICPP_IMGUI_CPP_H
