FILDEF void do_about ()
{
    set_ui_font(&get_editor_regular_font());

    begin_panel(WINDOW_BORDER,WINDOW_BORDER,get_viewport().w-(WINDOW_BORDER*2),get_viewport().h-(WINDOW_BORDER*2), UI_NONE, ui_color_ex_dark);
    begin_panel(1,1,get_viewport().w-2,get_viewport().h-2, UI_NONE, ui_color_medium);

    constexpr float XPAD = 4;
    constexpr float YPAD = 4;

    vec2 cursor(XPAD, YPAD);

    set_panel_cursor_dir(UI_DIR_DOWN);
    set_panel_cursor(&cursor);

    #if defined(BUILD_DEBUG)
    const char* build = "Debug";
    #else
    const char* build = "Release";
    #endif

    float lw = get_viewport().w - (XPAD*2);
    float lh = 18;

    constexpr const char* CHANGELOG = "https://github.com/thatb0y/tein-editor/blob/master/CHANGES";
    std::string changelog_url(format_string("%s#v%d.%d.%d", CHANGELOG, APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH));

    do_label_hyperlink(UI_ALIGN_LEFT,UI_ALIGN_CENTER, lw,lh, format_string("The End is Nigh Editor %s (v%d.%d.%d) %s Build ", APP_ARCH, APP_VER_MAJOR,APP_VER_MINOR,APP_VER_PATCH, build), "(Changelog)", changelog_url);
    do_label          (UI_ALIGN_LEFT,UI_ALIGN_CENTER, lw,lh, "Copyright (C) 2020 Joshua Robertson");
    advance_panel_cursor(lh/2);
    do_label_hyperlink(UI_ALIGN_LEFT,UI_ALIGN_CENTER, lw,lh, "Flame icon made by Those Icons from ", "Flaticon", "https://www.flaticon.com/");
    do_label          (UI_ALIGN_LEFT,UI_ALIGN_CENTER, lw,lh, "Editor icon made by Radley McTuneston");

    end_panel();
    end_panel();
}

FILDEF void handle_about_events ()
{
    if (!is_window_focused("About")) return;

    if (main_event.type == SDL_KEYDOWN)
    {
        if (main_event.key.keysym.sym == SDLK_ESCAPE ||
            main_event.key.keysym.sym == SDLK_RETURN)
        {
            hide_window("Abour");
        }
    }
}
