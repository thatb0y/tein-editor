GLOBAL const vec4 GPAK_PROGRESS_BAR_MIN_COLOR = { .2f,.5f,.2f, 1 };
GLOBAL const vec4 GPAK_PROGRESS_BAR_MAX_COLOR = { .2f,.9f,.2f, 1 };

GLOBAL SDL_Thread* gpak_unpack_thread;
GLOBAL SDL_Thread* gpak_pack_thread;

struct GPAK_Unpack_Data
{
    std::string file_name;
    bool overwrite;

    std::atomic<float> progress; // In percent.
    std::atomic<bool>  complete = true; // Default to true so we can press the unpack button.
    std::atomic<bool>  cancel;

    std::atomic<GPAK_Error> error;
};

struct GPAK_Pack_Data
{
    std::string file_name;
    std::vector<std::string> paths;

    std::atomic<float> progress; // In percent.
    std::atomic<bool>  complete = true; // Default to true so we can press the pack button.
    std::atomic<bool>  cancel;

    std::atomic<GPAK_Error> error;
};

GLOBAL GPAK_Unpack_Data gpak_unpack_data;
GLOBAL GPAK_Pack_Data gpak_pack_data;

FILDEF void internal__handle_gpak_error (GPAK_Error error)
{
    switch (error)
    {
        case(GPAK_Error::WRITE): LOG_ERROR(ERR_MED, "Failed to write GPAK data!"); break;
        case(GPAK_Error::READ ): LOG_ERROR(ERR_MED, "Failed to read GPAK data!" ); break;
        case(GPAK_Error::EMPTY): LOG_ERROR(ERR_MED, "No files found to pack!"   ); break;
    }
}

STDDEF int internal__gpak_unpack_thread_main (void* user_data)
{
    GPAK_Unpack_Data* args = CAST(GPAK_Unpack_Data*, user_data);

    std::string file_name = args->file_name;
    bool overwrite = args->overwrite;

    FILE* file = fopen(file_name.c_str(), "rb");
    if (!file)
    {
        args->error.store(GPAK_Error::READ);
        return EXIT_FAILURE;
    }
    defer { fclose(file); };

    std::vector<GPAK_Entry> entries;
    u32 entry_count;

    fread(&entry_count, sizeof(u32), 1, file);
    entries.resize(entry_count);

    for (auto& e: entries)
    {
        fread(&e.name_length, sizeof(u16),  1,             file);
        e.name.resize(e.name_length);
        fread(&e.name[0],     sizeof(char), e.name_length, file);
        fread(&e.file_size,   sizeof(u32),  1,             file);

        if (args->cancel.load()) // Cancel!
        {
            return EXIT_SUCCESS;
        }
    }

    size_t max_size = std::max_element(
    entries.begin(), entries.end(),
    [] (const GPAK_Entry& a, const GPAK_Entry& b)
    {
        return (a.file_size < b.file_size);
    }
    )->file_size;

    std::vector<u8> file_buffer;
    file_buffer.resize(max_size);

    u32 entries_unpacked = 0;

    std::string base_path(strip_file_name(file_name));
    for (auto& e: entries)
    {
        fread(&file_buffer[0], sizeof(u8), e.file_size, file);

        std::string full_file_name(base_path + e.name);
        std::string full_file_path(strip_file_name(full_file_name));

        if (!does_path_exist(full_file_path))
        {
            create_path(full_file_path);
        }
        if (!does_file_exist(full_file_name) || overwrite)
        {
            FILE* output = fopen(full_file_name.c_str(), "wb");
            if (output)
            {
                fwrite(&file_buffer[0], sizeof(u8), e.file_size, output);
                fclose(output);
            }
        }

        ++entries_unpacked;
        args->progress.store(CAST(float, entries_unpacked) / CAST(float, entry_count));
        push_editor_event(EDITOR_EVENT_GPAK_PROGRESS, NULL, NULL);

        if (args->cancel.load()) // Cancel!
        {
            return EXIT_SUCCESS;
        }
    }

    args->complete.store(true);
    push_editor_event(EDITOR_EVENT_GPAK_PROGRESS, NULL, NULL);

    // printf("GPAK Unpack Complete!\n");
    return EXIT_SUCCESS;
}

STDDEF int internal__gpak_pack_thread_main (void* user_data)
{
    GPAK_Pack_Data* args = CAST(GPAK_Pack_Data*, user_data);

    std::string file_name = args->file_name;
    std::vector<std::string> paths = args->paths;

    // We say approximate because this isn't true if you are loading the loader
    // GPAK or if it is a modified GPAK with extra stuff in (added mod files).
    GLOBAL constexpr size_t APRROX_TEIN_GPAK_ENTRIES = 1340;

    FILE* file = fopen(file_name.c_str(), "wb");
    if (!file)
    {
        args->error.store(GPAK_Error::WRITE);
        return EXIT_FAILURE;
    }
    defer { fclose(file); };

    std::vector<std::string> files;
    std::vector<std::string> stripped_files;

    files.reserve(APRROX_TEIN_GPAK_ENTRIES);
    stripped_files.reserve(APRROX_TEIN_GPAK_ENTRIES);

    for (auto path: paths)
    {
        std::vector<std::string> temp_files;
        list_path_files(path, temp_files, true);

        files.insert(files.end(), temp_files.begin(), temp_files.end());

        std::string stripped_path(path.substr(0, path.find_last_of("/\\")+1));
        for (auto& f: temp_files)
        {
            f = f.substr(stripped_path.length());
        }
        stripped_files.insert(stripped_files.end(), temp_files.begin(), temp_files.end());
    }

    if (files.empty())
    {
        args->error.store(GPAK_Error::EMPTY);
        return EXIT_FAILURE;
    }

    ASSERT(files.size() == stripped_files.size());

    u32 entry_count = CAST(u32, files.size());
    fwrite(&entry_count, sizeof(u32), 1, file);

    u32 entries_packed = 0;

    size_t max_size = 0;
    for (size_t i=0; i<files.size(); ++i)
    {
        std::string name = stripped_files.at(i);
        u16 name_length  = CAST(u16, name.length());
        u32 file_size    = CAST(u32, get_size_of_file(files.at(i)));

        max_size = std::max(max_size, CAST(size_t, file_size));

        fwrite(&name_length, sizeof(u16),  1,           file);
        fwrite(name.c_str(), sizeof(char), name_length, file);
        fwrite(&file_size,   sizeof(u32),  1,           file);

        ++entries_packed;
        args->progress.store(((CAST(float, entries_packed) / CAST(float, entry_count)) / 2));
        push_editor_event(EDITOR_EVENT_GPAK_PROGRESS, NULL, NULL);

        if (args->cancel.load()) // Cancel!
        {
            return EXIT_SUCCESS;
        }
    }

    std::vector<u8> file_buffer;
    file_buffer.resize(max_size);

    u32 file_count = CAST(u32, files.size());
    u32 files_packed = 0;

    for (auto& f: files)
    {
        FILE* input = fopen(f.c_str(), "rb");
        if (input)
        {
            size_t file_size = get_size_of_file(input);
            fread (&file_buffer[0], sizeof(u8), file_size, input);
            fwrite(&file_buffer[0], sizeof(u8), file_size, file);
            fclose(input);
        }

        ++files_packed;
        args->progress.store(.5f + ((CAST(float, files_packed) / CAST(float, file_count)) / 2));
        push_editor_event(EDITOR_EVENT_GPAK_PROGRESS, NULL, NULL);

        if (args->cancel.load()) // Cancel!
        {
            return EXIT_SUCCESS;
        }
    }

    args->complete.store(true);
    push_editor_event(EDITOR_EVENT_GPAK_PROGRESS, NULL, NULL);

    return EXIT_SUCCESS;
}

FILDEF void gpak_unpack (std::string file_name, bool overwrite)
{
    gpak_unpack_data.file_name = file_name;
    gpak_unpack_data.overwrite = overwrite;
    gpak_unpack_data.progress  = 0;
    gpak_unpack_data.complete  = false;
    gpak_unpack_data.cancel    = false;
    gpak_unpack_data.error     = GPAK_Error::NONE;

    gpak_unpack_thread = SDL_CreateThread(internal__gpak_unpack_thread_main, "UnpackGPAK", &gpak_unpack_data);
    if (!gpak_unpack_thread)
    {
        LOG_ERROR(ERR_MED, "Failed to perform GPAK unpack operation! (%s)", SDL_GetError());
        return;
    }
    SDL_DetachThread(gpak_unpack_thread);

    show_window("Unpack");
}

FILDEF void gpak_pack (std::string file_name, std::vector<std::string> paths)
{
    gpak_pack_data.file_name = file_name;
    gpak_pack_data.paths     = paths;
    gpak_pack_data.progress  = 0;
    gpak_pack_data.complete  = false;
    gpak_pack_data.cancel    = false;
    gpak_pack_data.error     = GPAK_Error::NONE;

    gpak_pack_thread = SDL_CreateThread(internal__gpak_pack_thread_main, "PackGPAK", &gpak_pack_data);
    if (!gpak_pack_thread)
    {
        LOG_ERROR(ERR_MED, "Failed to perform GPAK pack operation! (%s)", SDL_GetError());
        return;
    }
    SDL_DetachThread(gpak_pack_thread);

    show_window("Pack");
}

FILDEF float gpak_unpack_progress ()
{
    return gpak_unpack_data.progress.load();
}
FILDEF float gpak_pack_progress ()
{
    return gpak_pack_data.progress.load();
}

FILDEF bool is_gpak_unpack_complete ()
{
    return gpak_unpack_data.complete.load();
}
FILDEF bool is_gpak_pack_complete ()
{
    return gpak_pack_data.complete.load();
}

FILDEF void do_unpack ()
{
    if (is_window_hidden("Unpack")) return;

    quad p1, p2;

    p1.x = WINDOW_BORDER;
    p1.y = WINDOW_BORDER;
    p1.w = get_viewport().w - (WINDOW_BORDER * 2);
    p1.h = get_viewport().h - (WINDOW_BORDER * 2);

    set_ui_font(&get_editor_regular_font());

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    float vw = get_viewport().w;
    float vh = get_viewport().h;

    p2.x =             1;
    p2.y =             1;
    p2.w = vw        - 2;
    p2.h = vh - p2.y - 1;

    begin_panel(p2, UI_NONE, ui_color_medium);

    constexpr float XPAD = 8;
    constexpr float YPAD = 4;

    vec2 cursor(XPAD, YPAD);

    set_panel_cursor_dir(UI_DIR_DOWN);
    set_panel_cursor(&cursor);

    constexpr float LABEL_H = 24;
    constexpr float BAR_H = 20;

    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, LABEL_H, "Unpacking GPAK...");

    cursor.y += (YPAD*2);

    float total_width = get_viewport().w - (XPAD*2);
    float current_width = total_width * gpak_unpack_progress();

    float x1 = cursor.x;
    float y1 = cursor.y;
    float x2 = cursor.x + current_width;
    float y2 = cursor.y + BAR_H;

    set_draw_color(ui_color_light);
    fill_quad(x1-2, y1-2, cursor.x+total_width+2, y2+2);
    set_draw_color(ui_color_ex_dark);
    fill_quad(x1-1, y1-1, cursor.x+total_width+1, y2+1);

    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(x1, y2, GPAK_PROGRESS_BAR_MIN_COLOR); // BL
    put_vertex(x1, y1, GPAK_PROGRESS_BAR_MIN_COLOR); // TL
    put_vertex(x2, y2, GPAK_PROGRESS_BAR_MAX_COLOR); // BR
    put_vertex(x2, y1, GPAK_PROGRESS_BAR_MAX_COLOR); // TR
    end_draw();

    end_panel();
    end_panel();

    if (is_gpak_unpack_complete())
    {
        internal__handle_gpak_error(gpak_unpack_data.error.load());
        hide_window("Unpack");
    }
}

FILDEF void do_pack ()
{
    if (is_window_hidden("Pack")) return;

    quad p1, p2;

    p1.x = WINDOW_BORDER;
    p1.y = WINDOW_BORDER;
    p1.w = get_viewport().w - (WINDOW_BORDER * 2);
    p1.h = get_viewport().h - (WINDOW_BORDER * 2);

    set_ui_font(&get_editor_regular_font());

    begin_panel(p1, UI_NONE, ui_color_ex_dark);

    float vw = get_viewport().w;
    float vh = get_viewport().h;

    p2.x =             1;
    p2.y =             1;
    p2.w = vw        - 2;
    p2.h = vh - p2.y - 1;

    begin_panel(p2, UI_NONE, ui_color_medium);

    constexpr float XPAD = 8;
    constexpr float YPAD = 4;

    vec2 cursor(XPAD, YPAD);

    set_panel_cursor_dir(UI_DIR_DOWN);
    set_panel_cursor(&cursor);

    constexpr float LABEL_H = 24;
    constexpr float BAR_H = 20;

    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, LABEL_H, "Packing GPAK...");

    cursor.y += (YPAD*2);

    float total_width = get_viewport().w - (XPAD*2);
    float current_width = total_width * gpak_pack_progress();

    float x1 = cursor.x;
    float y1 = cursor.y;
    float x2 = cursor.x + current_width;
    float y2 = cursor.y + BAR_H;

    set_draw_color(ui_color_light);
    fill_quad(x1-2, y1-2, cursor.x+total_width+2, y2+2);
    set_draw_color(ui_color_ex_dark);
    fill_quad(x1-1, y1-1, cursor.x+total_width+1, y2+1);

    begin_draw(Buffer_Mode::TRIANGLE_STRIP);
    put_vertex(x1, y2, GPAK_PROGRESS_BAR_MIN_COLOR); // BL
    put_vertex(x1, y1, GPAK_PROGRESS_BAR_MIN_COLOR); // TL
    put_vertex(x2, y2, GPAK_PROGRESS_BAR_MAX_COLOR); // BR
    put_vertex(x2, y1, GPAK_PROGRESS_BAR_MAX_COLOR); // TR
    end_draw();

    end_panel();
    end_panel();

    if (is_gpak_pack_complete())
    {
        internal__handle_gpak_error(gpak_pack_data.error.load());
        hide_window("Pack");
    }
}

FILDEF void cancel_unpack ()
{
    gpak_unpack_data.complete.store(true);
    gpak_unpack_data.cancel.store(true);

    hide_window("Unpack");
}

FILDEF void cancel_pack ()
{
    gpak_pack_data.complete.store(true);
    gpak_pack_data.cancel.store(true);

    hide_window("Pack");
}
