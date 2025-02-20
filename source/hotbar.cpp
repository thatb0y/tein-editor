GLOBAL constexpr const char* HB_NAME_NEW         = "New";
GLOBAL constexpr const char* HB_NAME_LOAD        = "Load";
GLOBAL constexpr const char* HB_NAME_SAVE        = "Save";
GLOBAL constexpr const char* HB_NAME_SAVE_AS     = "Save As";
GLOBAL constexpr const char* HB_NAME_UNDO        = "Undo";
GLOBAL constexpr const char* HB_NAME_REDO        = "Redo";
GLOBAL constexpr const char* HB_NAME_ZOOM_OUT    = "Zoom Out";
GLOBAL constexpr const char* HB_NAME_ZOOM_IN     = "Zoom In";
GLOBAL constexpr const char* HB_NAME_PACK        = "Pack GPAK";
GLOBAL constexpr const char* HB_NAME_UNPACK      = "Unpack GPAK";
GLOBAL constexpr const char* HB_NAME_RUN_GAME    = "Play";
GLOBAL constexpr const char* HB_NAME_PREFERENCES = "Preferences";
GLOBAL constexpr const char* HB_NAME_ABOUT       = "About";
GLOBAL constexpr const char* HB_NAME_HELP        = "Help";
GLOBAL constexpr const char* HB_NAME_BUG_REPORT  = "Bug Report";
GLOBAL constexpr const char* HB_NAME_BACKUPS     = "Backups";

GLOBAL constexpr const char* HB_INFO_NEW         = "Create a new empty level.";
GLOBAL constexpr const char* HB_INFO_LOAD        = "Load an existing level.";
GLOBAL constexpr const char* HB_INFO_SAVE        = "Save the current level.";
GLOBAL constexpr const char* HB_INFO_SAVE_AS     = "Save the current level as a new file.";
GLOBAL constexpr const char* HB_INFO_UNDO        = "Undo the last recorded action.";
GLOBAL constexpr const char* HB_INFO_REDO        = "Redo the last recorded action.";
GLOBAL constexpr const char* HB_INFO_ZOOM_OUT    = "Zoom out the editor camera.";
GLOBAL constexpr const char* HB_INFO_ZOOM_IN     = "Zoom in the editor camera.";
GLOBAL constexpr const char* HB_INFO_PACK        = "Pack data files into the GPAK file format.";
GLOBAL constexpr const char* HB_INFO_UNPACK      = "Unpack data files from the GPAK file format.";
GLOBAL constexpr const char* HB_INFO_RUN_GAME    = "Runs The End is Nigh game application.";
GLOBAL constexpr const char* HB_INFO_PREFERENCES = "Open the preferences menu to customize the editor.";
GLOBAL constexpr const char* HB_INFO_ABOUT       = "Open the about menu for application information.";
GLOBAL constexpr const char* HB_INFO_BUG_REPORT  = "Report technical issues or bugs with the editor.";
GLOBAL constexpr const char* HB_INFO_HELP        = "Information and help about modding The End is Nigh.";
GLOBAL constexpr const char* HB_INFO_BACKUPS     = "Open the folder containing level and map backups.";

FILDEF void do_hotbar ()
{
    vec2 cursor(0,0);

    // Height for the toolbar buttons.
    float bh = HOTBAR_HEIGHT - WINDOW_BORDER;

    set_ui_texture(&resource_icons);
    set_ui_font(&get_editor_regular_font());

    UI_Flag save_flags     = UI_NONE;
    UI_Flag save_as_flags  = UI_NONE;
    UI_Flag undo_flags     = UI_NONE;
    UI_Flag redo_flags     = UI_NONE;
    UI_Flag zoom_out_flags = UI_NONE;
    UI_Flag zoom_in_flags  = UI_NONE;
    UI_Flag pack_flags     = UI_NONE;
    UI_Flag unpack_flags   = UI_NONE;

    pack_flags   = (is_gpak_pack_complete  ()) ? UI_NONE : UI_LOCKED;
    unpack_flags = (is_gpak_unpack_complete()) ? UI_NONE : UI_LOCKED;

    if (are_there_any_tabs())
    {
        const Tab& tab = get_current_tab();

        if (tab.type == Tab_Type::LEVEL)
        {
            if (tab.level_history.current_position == CAST(int, tab.level_history.state.size())-1)
            {
                redo_flags = UI_LOCKED;
            }
            if (tab.level_history.current_position <= -1)
            {
                undo_flags = UI_LOCKED;
            }
            if (tab.level_history.state.empty())
            {
                undo_flags = UI_LOCKED;
                redo_flags = UI_LOCKED;
            }
        }
        else
        {
            if (tab.map_history.current_position == CAST(int, tab.map_history.state.size())-1)
            {
                redo_flags = UI_LOCKED;
            }
            if (tab.map_history.current_position <= 0)
            {
                undo_flags = UI_LOCKED;
            }
            if (tab.map_history.state.size() == 1)
            {
                undo_flags = UI_LOCKED;
                redo_flags = UI_LOCKED;
            }
        }

        if (tab.type == Tab_Type::LEVEL)
        {
            zoom_out_flags = (tab.camera.zoom == MIN_LVL_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
            zoom_in_flags  = (tab.camera.zoom == MAX_LVL_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
        }
        else
        {
            zoom_out_flags = (tab.camera.zoom == MIN_MAP_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
            zoom_in_flags  = (tab.camera.zoom == MAX_MAP_EDITOR_ZOOM) ? UI_LOCKED : UI_NONE;
        }
    }
    else
    {
        save_flags     = UI_LOCKED;
        save_as_flags  = UI_LOCKED;
        undo_flags     = UI_LOCKED;
        redo_flags     = UI_LOCKED;
        zoom_out_flags = UI_LOCKED;
        zoom_in_flags  = UI_LOCKED;
    }

    // The hotbar is a horizontal list of available actions.
    begin_panel(0, 0, get_viewport().w, HOTBAR_HEIGHT, UI_NONE, ui_color_medium);

    set_panel_cursor(&cursor);
    set_panel_cursor_dir(UI_DIR_RIGHT);

    // We want these to be the same width as the toolbar (looks nice).
    float bw = TOOLBAR_WIDTH+1;

    float width = 0;

    width += calculate_button_txt_width(HB_NAME_NEW        );
    width += calculate_button_txt_width(HB_NAME_LOAD       );
    width += calculate_button_txt_width(HB_NAME_SAVE       );
    width += calculate_button_txt_width(HB_NAME_SAVE_AS    );
    width += calculate_button_txt_width(HB_NAME_UNDO       );
    width += calculate_button_txt_width(HB_NAME_REDO       );
    width += calculate_button_txt_width(HB_NAME_ZOOM_OUT   );
    width += calculate_button_txt_width(HB_NAME_ZOOM_IN    );
    width += calculate_button_txt_width(HB_NAME_PACK       );
    width += calculate_button_txt_width(HB_NAME_UNPACK     );
    width += calculate_button_txt_width(HB_NAME_RUN_GAME   );
    width += calculate_button_txt_width(HB_NAME_PREFERENCES);
    width += calculate_button_txt_width(HB_NAME_ABOUT      );
    width += calculate_button_txt_width(HB_NAME_BUG_REPORT );
    width += calculate_button_txt_width(HB_NAME_BACKUPS    );
    width += calculate_button_txt_width(HB_NAME_HELP       );

    // Display text or icons depending on what we have room for.
    if (width < get_viewport().w)
    {
    do_button_txt(hb_new,             bh,  UI_NONE,         HB_NAME_NEW,          HB_INFO_NEW,          KB_LEVEL_NEW                            );
    do_button_txt(hb_load,            bh,  UI_NONE,         HB_NAME_LOAD,         HB_INFO_LOAD,         KB_LEVEL_OPEN                           );
    do_button_txt(hb_save,            bh,  save_flags,      HB_NAME_SAVE,         HB_INFO_SAVE,         KB_LEVEL_SAVE                           );
    do_button_txt(hb_save_as,         bh,  save_as_flags,   HB_NAME_SAVE_AS,      HB_INFO_SAVE_AS,      KB_LEVEL_SAVE_AS                        );
    do_button_txt(hb_undo_action,     bh,  undo_flags,      HB_NAME_UNDO,         HB_INFO_UNDO,         KB_UNDO                                 );
    do_button_txt(hb_redo_action,     bh,  redo_flags,      HB_NAME_REDO,         HB_INFO_REDO,         KB_REDO                                 );
    do_button_txt(hb_zoom_out,        bh,  zoom_out_flags,  HB_NAME_ZOOM_OUT,     HB_INFO_ZOOM_OUT,     KB_CAMERA_ZOOM_OUT                      );
    do_button_txt(hb_zoom_in,         bh,  zoom_in_flags,   HB_NAME_ZOOM_IN,      HB_INFO_ZOOM_IN,      KB_CAMERA_ZOOM_IN                       );
    do_button_txt(hb_gpak_pack,       bh,  pack_flags,      HB_NAME_PACK,         HB_INFO_PACK,         KB_GPAK_PACK                            );
    do_button_txt(hb_gpak_unpack,     bh,  unpack_flags,    HB_NAME_UNPACK,       HB_INFO_UNPACK,       KB_GPAK_UNPACK                          );
    do_button_txt(hb_run_game,        bh,  UI_NONE,         HB_NAME_RUN_GAME,     HB_INFO_RUN_GAME,     KB_RUN_GAME                             );
    do_button_txt(hb_preferences,     bh,  UI_NONE,         HB_NAME_PREFERENCES,  HB_INFO_PREFERENCES,  KB_PREFERENCES                          );
    do_button_txt(hb_about,           bh,  UI_NONE,         HB_NAME_ABOUT,        HB_INFO_ABOUT,        KB_ABOUT                                );
    do_button_txt(hb_bug_report,      bh,  UI_NONE,         HB_NAME_BUG_REPORT,   HB_INFO_BUG_REPORT,   KB_BUG_REPORT                           );
    do_button_txt(hb_backups,         bh,  UI_NONE,         HB_NAME_BACKUPS,      HB_INFO_BACKUPS,      KB_BACKUPS                              );
    do_button_txt(hb_help,            bh,  UI_NONE,         HB_NAME_HELP,         HB_INFO_HELP,         KB_HELP                                 );
    }
    else
    {
    do_button_img(hb_new,          bw,bh,  UI_NONE,         &CLIP_NEW,            HB_INFO_NEW,          KB_LEVEL_NEW,        HB_NAME_NEW        );
    do_button_img(hb_load,         bw,bh,  UI_NONE,         &CLIP_LOAD,           HB_INFO_LOAD,         KB_LEVEL_OPEN,       HB_NAME_LOAD       );
    do_button_img(hb_save,         bw,bh,  save_flags,      &CLIP_SAVE,           HB_INFO_SAVE,         KB_LEVEL_SAVE,       HB_NAME_SAVE       );
    do_button_img(hb_save,         bw,bh,  save_as_flags,   &CLIP_SAVE_AS,        HB_INFO_SAVE_AS,      KB_LEVEL_SAVE_AS,    HB_NAME_SAVE_AS    );
    do_button_img(hb_undo_action,  bw,bh,  undo_flags,      &CLIP_UNDO,           HB_INFO_UNDO,         KB_UNDO,             HB_NAME_UNDO       );
    do_button_img(hb_redo_action,  bw,bh,  redo_flags,      &CLIP_REDO,           HB_INFO_REDO,         KB_REDO,             HB_NAME_REDO       );
    do_button_img(hb_zoom_out,     bw,bh,  zoom_out_flags,  &CLIP_ZOOM_OUT,       HB_INFO_ZOOM_OUT,     KB_CAMERA_ZOOM_OUT,  HB_NAME_ZOOM_OUT   );
    do_button_img(hb_zoom_in,      bw,bh,  zoom_in_flags,   &CLIP_ZOOM_IN,        HB_INFO_ZOOM_IN,      KB_CAMERA_ZOOM_IN,   HB_NAME_ZOOM_IN    );
    do_button_img(hb_gpak_pack,    bw,bh,  pack_flags,      &CLIP_PACK,           HB_INFO_PACK,         KB_GPAK_PACK,        HB_NAME_PACK       );
    do_button_img(hb_gpak_unpack,  bw,bh,  unpack_flags,    &CLIP_UNPACK,         HB_INFO_UNPACK,       KB_GPAK_UNPACK,      HB_NAME_UNPACK     );
    do_button_img(hb_run_game,     bw,bh,  UI_NONE,         &CLIP_RUN,            HB_INFO_RUN_GAME,     KB_RUN_GAME,         HB_NAME_RUN_GAME   );
    do_button_img(hb_preferences,  bw,bh,  UI_NONE,         &CLIP_SETTINGS,       HB_INFO_PREFERENCES,  KB_PREFERENCES,      HB_NAME_PREFERENCES);
    do_button_img(hb_about,        bw,bh,  UI_NONE,         &CLIP_ABOUT,          HB_INFO_ABOUT,        KB_ABOUT,            HB_NAME_ABOUT      );
    do_button_img(hb_bug_report,   bw,bh,  UI_NONE,         &CLIP_BUG,            HB_INFO_BUG_REPORT,   KB_BUG_REPORT,       HB_NAME_BUG_REPORT );
    do_button_img(hb_backups,      bw,bh,  UI_NONE,         &CLIP_BACKUPS,        HB_INFO_BACKUPS,      KB_BACKUPS,          HB_NAME_BACKUPS    );
    do_button_img(hb_help,         bw,bh,  UI_NONE,         &CLIP_HELP,           HB_INFO_HELP,         KB_HELP,             HB_NAME_HELP       );
    }

    end_panel();
}

FILDEF void hb_new ()
{
    open_new();
}

FILDEF void hb_load ()
{
    std::vector<std::string> file_names = open_dialog(Dialog_Type::LVL_CSV);
    if (!file_names.empty())
    {
        for (auto file: file_names)
        {
            std::string ext(file.substr(file.find_last_of(".")));
            if      (ext == ".lvl") load_level_tab(file);
            else if (ext == ".csv") load_map_tab  (file);
        }
    }
}

FILDEF void hb_save ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_save     (get_current_tab()); break;
        case (Tab_Type::MAP  ): save_map_tab(get_current_tab()); break;
    }
}

FILDEF void hb_save_as ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_save_as     (); break;
        case (Tab_Type::MAP  ): save_map_tab_as(); break;
    }
}

FILDEF void hb_undo_action ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_undo(); break;
        case (Tab_Type::MAP  ): me_undo(); break;
    }
}

FILDEF void hb_redo_action ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_redo(); break;
        case (Tab_Type::MAP  ): me_redo(); break;
    }
}

FILDEF void hb_history_begin ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_history_begin(); break;
        case (Tab_Type::MAP  ): me_history_begin(); break;
    }
}

FILDEF void hb_history_end ()
{
    if (!are_there_any_tabs()) return;

    switch (get_current_tab().type)
    {
        case (Tab_Type::LEVEL): le_history_end(); break;
        case (Tab_Type::MAP  ): me_history_end(); break;
    }
}

FILDEF void hb_zoom_out ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();

    if (tab.type == Tab_Type::LEVEL)
    {
        if ((tab.camera.zoom /= 2) < MIN_LVL_EDITOR_ZOOM)
        {
            tab.camera.zoom = MIN_LVL_EDITOR_ZOOM;
        }
    }
    else
    {
        if ((tab.camera.zoom /= 2) < MIN_MAP_EDITOR_ZOOM)
        {
            tab.camera.zoom = MIN_MAP_EDITOR_ZOOM;
        }
    }
}

FILDEF void hb_zoom_in ()
{
    if (!are_there_any_tabs()) return;

    Tab& tab = get_current_tab();

    if (tab.type == Tab_Type::LEVEL)
    {
        if ((tab.camera.zoom *= 2) > MAX_LVL_EDITOR_ZOOM)
        {
            tab.camera.zoom = MAX_LVL_EDITOR_ZOOM;
        }
    }
    else
    {
        if ((tab.camera.zoom *= 2) > MAX_MAP_EDITOR_ZOOM)
        {
            tab.camera.zoom = MAX_MAP_EDITOR_ZOOM;
        }
    }
}

FILDEF void hb_gpak_unpack ()
{
    std::vector<std::string> files = open_dialog(Dialog_Type::GPAK, false);
    if (!files.empty())
    {
        bool should_overwrite = true;
        if (show_alert("Overwrite", "Do you want to overwrite any existing files during unpack?",
                ALERT_TYPE_INFO, ALERT_BUTTON_YES_NO) == ALERT_RESULT_NO)
        {
            should_overwrite = false;
        }

        gpak_unpack(files.at(0), should_overwrite);
    }
}

FILDEF void hb_gpak_pack ()
{
    std::vector<std::string> paths = path_dialog();
    if (!paths.empty())
    {
        std::string file = save_dialog(Dialog_Type::GPAK);
        if (!file.empty())
        {
            gpak_pack(file, paths);
        }
    }
}

FILDEF void hb_run_game ()
{
    constexpr const char* EXE_STEAM_X86 = "C:/Program Files (x86)/Steam/steamapps/common/theendisnigh/TheEndIsNigh.exe";
    constexpr const char* EXE_STEAM_X64 = "C:/Program Files/Steam/steamapps/common/theendisnigh/TheEndIsNigh.exe";
    constexpr const char* EXE_STEAM_APP = "TheEndIsNigh.exe";

    constexpr const char* EXE_EPIC_X86  = "C:/Program Files (x86)/Epic Games/theendisnigh/TheEnd.exe";
    constexpr const char* EXE_EPIC_X64  = "C:/Program Files)/Epic Games/theendisnigh/TheEnd.exe";
    constexpr const char* EXE_EPIC_APP  = "TheEnd.exe";

    const std::vector<std::string> EXECUTABLES
    {
        EXE_STEAM_X86, EXE_STEAM_X64, EXE_STEAM_APP,
        EXE_EPIC_X86, EXE_EPIC_X64, EXE_EPIC_APP
    };

    std::string executable;
    if (!editor_settings.game_path.empty())
    {
        executable = editor_settings.game_path;
        if (!does_file_exist(executable))
        {
            executable.clear();
        }
    }
    if (executable.empty())
    {
        for (auto exe: EXECUTABLES)
        {
            executable = exe;
            if (does_file_exist(executable))
            {
                break;
            }
        }
        if (!does_file_exist(executable))
        {
            executable.clear();
        }
    }

    // Executable couldn't be found so we will ask for the location.
    if (executable.empty())
    {
        open_path();
    }
    else if (!run_executable(executable))
    {
        LOG_ERROR(ERR_MED, "Failed to launch The End is Nigh executable!");
    }
}

FILDEF void hb_preferences ()
{
    if (is_window_hidden("Preferences"))
    {
        init_preferences_menu(); // Load current settings.
        show_window("Preferences");
    }
    else
    {
        raise_window("Preferences");
    }
}

FILDEF void hb_about ()
{
    if (is_window_hidden("About"))
    {
        show_window("About");
    }
    else
    {
        raise_window("About");
    }
}

FILDEF void hb_help ()
{
    load_webpage("https://jrob774.github.io/tein-manual/");
}

FILDEF void hb_bug_report ()
{
    load_webpage("https://itch.io/t/822898/bug-reports");
}

FILDEF void hb_backups ()
{
    std::string backups_path(get_appdata_path() + BACKUPS_PATH);
    if (!does_path_exist(backups_path)) create_path(backups_path);
    if (does_path_exist(backups_path)) open_folder(backups_path);
}
