#pragma once

// These colors are filled with the appropriate values based on whether the
// user would prefer to use the dark or light theme for the level editor.

GLOBAL vec4 ui_color_black;
GLOBAL vec4 ui_color_ex_dark;
GLOBAL vec4 ui_color_dark;
GLOBAL vec4 ui_color_med_dark;
GLOBAL vec4 ui_color_medium;
GLOBAL vec4 ui_color_med_light;
GLOBAL vec4 ui_color_light;
GLOBAL vec4 ui_color_ex_light;
GLOBAL vec4 ui_color_white;

GLOBAL constexpr u32 UI_CURSOR_BLINK_INTERVAL = 700;

// The function type that is called by various UI widgets when triggered.
typedef void (*UI_Action)(void);

typedef u32 UI_Flag;

GLOBAL constexpr UI_Flag UI_NONE       = 0x00000000;
GLOBAL constexpr UI_Flag UI_LOCKED     = 0x00000001;
GLOBAL constexpr UI_Flag UI_INACTIVE   = 0x00000002;
GLOBAL constexpr UI_Flag UI_HIGHLIGHT  = 0x00000004;
GLOBAL constexpr UI_Flag UI_DARKEN     = 0x00000008;
GLOBAL constexpr UI_Flag UI_TOOLTIP    = 0x00000010;
GLOBAL constexpr UI_Flag UI_SINGLE     = 0x00000020;

// Flags for what type of characters can be written into a text box.
GLOBAL constexpr UI_Flag UI_ALPHABETIC = 0x00000040;
GLOBAL constexpr UI_Flag UI_NUMERIC    = 0x00000080;
GLOBAL constexpr UI_Flag UI_ALPHANUM   = 0x00000100;
GLOBAL constexpr UI_Flag UI_FILEPATH   = 0x00000200;

enum UI_Align
{
    UI_ALIGN_CENTER,
    UI_ALIGN_LEFT,
    UI_ALIGN_RIGHT,
    UI_ALIGN_TOP,
    UI_ALIGN_BOTTOM
};

enum UI_Dir
{
    UI_DIR_UP,
    UI_DIR_RIGHT,
    UI_DIR_DOWN,
    UI_DIR_LEFT
};

GLOBAL bool should_push_ui_redraw_event;

FILDEF bool init_ui_system   ();
FILDEF void load_ui_theme    ();
FILDEF void reset_ui_state   ();
FILDEF void handle_ui_events ();

FILDEF bool is_ui_light ();

FILDEF vec2 ui_get_relative_mouse ();

FILDEF bool mouse_in_ui_bounds_xywh (float x, float y, float w, float h);
FILDEF bool mouse_in_ui_bounds_xywh (quad b);

FILDEF void set_ui_texture (Texture* tex);
FILDEF void set_ui_font    (Font*    fnt);

FILDEF bool is_there_a_hot_ui_element ();
FILDEF bool is_there_a_hit_ui_element ();

FILDEF void deselect_active_text_box (std::string& text, std::string default_text);
FILDEF void deselect_active_text_box ();

FILDEF bool text_box_is_active ();

FILDEF bool hotkey_is_active              ();
FILDEF void deselect_active_hotkey_rebind ();

STDDEF void begin_panel (float x, float y, float w, float h, UI_Flag flags, vec4 c = vec4(0,0,0,0));
FILDEF void begin_panel (quad bounds, UI_Flag flags, vec4 c = vec4(0,0,0,0));

STDDEF bool begin_click_panel (UI_Action action, float w, float h, UI_Flag flags, std::string info = "");

STDDEF void end_panel ();

FILDEF float get_panel_w ();
FILDEF float get_panel_h ();

FILDEF vec2 get_panel_offset ();
FILDEF vec2 get_panel_cursor ();

FILDEF void disable_panel_cursor_advance ();
FILDEF void enable_panel_cursor_advance  ();

FILDEF void advance_panel_cursor (float advance);

FILDEF void set_panel_cursor     (vec2*  cursor);
FILDEF void set_panel_cursor_dir (UI_Dir    dir);

FILDEF void    set_panel_flags (UI_Flag flags);
FILDEF UI_Flag get_panel_flags ();

FILDEF float calculate_button_txt_width (std::string text);

STDDEF bool do_button_img (UI_Action action, float w, float h, UI_Flag flags, const quad* clip = NULL, std::string info = "", std::string kb = "", std::string name = "");
STDDEF bool do_button_txt (UI_Action action, float w, float h, UI_Flag flags, std::string text,        std::string info = "", std::string kb = "", std::string name = "");
FILDEF bool do_button_txt (UI_Action action,          float h, UI_Flag flags, std::string text,        std::string info = "", std::string kb = "", std::string name = "");

STDDEF void do_label (UI_Align horz, UI_Align vert, float w, float h, std::string text, vec4 bg = vec4(0,0,0,0));
FILDEF void do_label (UI_Align horz, UI_Align vert,          float h, std::string text, vec4 bg = vec4(0,0,0,0));

STDDEF void do_label_hyperlink (UI_Align horz, UI_Align vert, float w, float h, std::string text, std::string link, std::string href, vec4 bg = vec4(0,0,0,0));

// Doesn't care about the current UI font and uses the current editor font instead.
STDDEF void  do_markdown    (float w, float h, std::string text);
STDDEF float get_markdown_h (float w,          std::string text);

STDDEF void do_text_box         (float w, float h, UI_Flag flags, std::string& text,                                   std::string default_text = "", UI_Align halign = UI_ALIGN_RIGHT);
STDDEF void do_text_box_labeled (float w, float h, UI_Flag flags, std::string& text, float label_w, std::string label, std::string default_text = "", UI_Align halign = UI_ALIGN_RIGHT);

STDDEF void do_hotkey_rebind_main (float w, float h, UI_Flag flags, Key_Binding& kb);
STDDEF void do_hotkey_rebind_alt  (float w, float h, UI_Flag flags, Key_Binding& kb);

STDDEF void do_icon (float w, float h, Texture& tex, const quad* clip = NULL);
FILDEF void do_quad (float w, float h, vec4 color);

FILDEF void do_separator (float size);

FILDEF void do_scrollbar (quad bounds,                        float content_height, float& scroll_offset);
STDDEF void do_scrollbar (float x, float y, float w, float h, float content_height, float& scroll_offset);

STDDEF void begin_panel_gradient (float x, float y, float w, float h, UI_Flag flags, vec4 cl = vec4(0,0,0,0), vec4 cr = vec4(0,0,0,0));
FILDEF void begin_panel_gradient (quad bounds,                        UI_Flag flags, vec4 cl = vec4(0,0,0,0), vec4 cr = vec4(0,0,0,0));

STDDEF bool begin_click_panel_gradient (UI_Action action, float w, float h, UI_Flag flags, std::string info = "");

STDDEF bool do_button_img_gradient (UI_Action action, float w, float h, UI_Flag flags, const quad* clip = NULL, std::string info = "", std::string kb = "", std::string name = "");
