GLOBAL constexpr const char* RESOURCE_LOCATION = "resource_location.txt";
GLOBAL constexpr const char* RESOURCE_GPAK = "editor.gpak";

GLOBAL std::string resource_location;
GLOBAL std::string current_editor_font;

// Maps file names to the data that was stored within the editor GPAK.
GLOBAL std::map<std::string, std::vector<u8>> gpak_resource_lookup;

FILDEF bool init_resource_manager ()
{
    LOG_DEBUG("Looking for %s...", RESOURCE_GPAK);

    std::string gpak_file_name(make_path_absolute(RESOURCE_GPAK));
    if (!does_file_exist(gpak_file_name))
    {
        LOG_DEBUG("No %s found!", RESOURCE_GPAK);
        return true;
    }

    // Return true because we may still function.
    FILE* gpak = fopen(gpak_file_name.c_str(), "rb");
    if (!gpak)
    {
        LOG_ERROR(ERR_MED, "Failed to load %s!", RESOURCE_GPAK);
        return true;
    }
    defer { fclose(gpak); };

    std::vector<GPAK_Entry> entries;
    u32 entry_count;

    fread(&entry_count, sizeof(u32), 1, gpak);
    entries.resize(entry_count);

    for (auto& e: entries)
    {
        fread(&e.name_length, sizeof(u16),  1,             gpak);
        e.name.resize(e.name_length);
        fread(&e.name[0],     sizeof(char), e.name_length, gpak);
        fread(&e.file_size,   sizeof(u32),  1,             gpak);
    }

    std::vector<u8> file_buffer;
    for (auto& e: entries)
    {
        file_buffer.resize(e.file_size);
        fread(&file_buffer[0], sizeof(u8), e.file_size, gpak);
        gpak_resource_lookup.insert(std::pair<std::string, std::vector<u8>>(e.name, file_buffer));
    }

    LOG_DEBUG("Found %s!", RESOURCE_GPAK);
    return true;
}

FILDEF void get_resource_location ()
{
    std::string resource_location_file(get_executable_path() + RESOURCE_LOCATION);
    if (does_file_exist(resource_location_file))
    {
        std::string relative_path(read_entire_file(resource_location_file));
        resource_location = get_executable_path() + relative_path;
        // Remove trailing whitespace, if there is any.
        resource_location.erase(resource_location.find_last_not_of(" \t\n\r\f\v") + 1);
        LOG_DEBUG("Resource Location: %s", resource_location.c_str());
    }
}

// We attempt to load resources from file first, but if they don't exist
// then we fall-back to loading them from the editor's GPAK file instead.

FILDEF bool load_texture_resource (std::string file_name, Texture& tex, Texture_Wrap wrap)
{
    std::string abs_file_name(build_resource_string(file_name));
    bool result = false;
    if (does_file_exist(abs_file_name)) result = load_texture_from_file(tex, abs_file_name, wrap);
    else result = load_texture_from_data(tex, gpak_resource_lookup[file_name], wrap);
    if (!result) LOG_ERROR(ERR_MED, "Failed to load texture file: %s", abs_file_name.c_str());
    return result;
}

FILDEF bool load_atlas_resource (std::string file_name, Texture_Atlas& atlas)
{
    std::string abs_file_name(build_resource_string(file_name));
    bool result = false;
    if (does_file_exist(abs_file_name)) result = load_texture_atlas_from_file(atlas, abs_file_name);
    else result = load_texture_atlas_from_data(atlas, gpak_resource_lookup[file_name]);
    if (!result) LOG_ERROR(ERR_MED, "Failed to load texture atlas file: %s", abs_file_name.c_str());
    return result;
}

FILDEF bool load_font_resource (std::string file_name, Font& fnt, std::vector<int> pt, float csz)
{
    std::string abs_file_name(build_resource_string(file_name));
    bool result = false;
    if (does_file_exist(abs_file_name)) result = load_font_from_file(fnt, abs_file_name, pt, csz);
    else result = load_font_from_data(fnt, gpak_resource_lookup[file_name], pt, csz);
    if (!result) LOG_ERROR(ERR_MED, "Failed to load font file: %s", abs_file_name.c_str());
    return result;
}

FILDEF Shader load_shader_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (does_file_exist(abs_file_name)) return load_shader_from_file(abs_file_name);
    else return load_shader_from_data(gpak_resource_lookup[file_name]);
}

FILDEF std::vector<u8> load_binary_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (does_file_exist(abs_file_name)) return read_binary_file(abs_file_name);
    else return gpak_resource_lookup[file_name];
}

FILDEF SDL_Surface* load_surface_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (does_file_exist(abs_file_name)) return SDL_LoadBMP(abs_file_name.c_str());
    else return SDL_LoadBMP_RW(SDL_RWFromConstMem(&gpak_resource_lookup[file_name][0],
        CAST(int, gpak_resource_lookup[file_name].size())), true);
}

FILDEF std::string load_string_resource (std::string file_name)
{
    std::string abs_file_name(build_resource_string(file_name));
    if (does_file_exist(abs_file_name)) return read_entire_file(abs_file_name);
    else return std::string(gpak_resource_lookup[file_name].begin(),
        gpak_resource_lookup[file_name].end());
}

FILDEF bool load_editor_resources ()
{
    LOG_DEBUG("Loading editor resources...");

    if (!load_texture_resource("textures/editor_ui/tools.png", resource_icons))
    {
        LOG_ERROR(ERR_MAX, "Failed to load editor icons!");
        return false;
    }
    if (!load_texture_resource("textures/editor_ui/checker_x14.png", resource_checker_14, TEXTURE_WRAP_REPEAT))
    {
        LOG_ERROR(ERR_MAX, "Failed to load the checker-x14 image!");
        return false;
    }
    if (!load_texture_resource("textures/editor_ui/checker_x16.png", resource_checker_16, TEXTURE_WRAP_REPEAT))
    {
        LOG_ERROR(ERR_MAX, "Failed to load the checker-x16 image!");
        return false;
    }
    if (!load_texture_resource("textures/editor_ui/checker_x20.png", resource_checker_20, TEXTURE_WRAP_REPEAT))
    {
        LOG_ERROR(ERR_MAX, "Failed to load the checker-x20 image!");
        return false;
    }
    if (!load_font_resource("fonts/opensans-regular.ttf", resource_font_regular_sans, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LOG_ERROR(ERR_MAX, "Failed to load OpenSans regular font!");
        return false;
    }
    if (!load_font_resource("fonts/opensans-bold.ttf", resource_font_bold_sans, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LOG_ERROR(ERR_MAX, "Failed to load OpenSans bold font!");
        return false;
    }
    if (!load_font_resource("fonts/liberationmono-regular.ttf", resource_font_regular_libmono))
    {
        LOG_ERROR(ERR_MAX, "Failed to load LiberationMono regular font!");
        return false;
    }
    if (!load_font_resource("fonts/opendyslexic-regular.ttf", resource_font_regular_dyslexic, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LOG_ERROR(ERR_MAX, "Failed to load OpenDyslexic regular font!");
        return false;
    }
    if (!load_font_resource("fonts/opendyslexic-bold.ttf", resource_font_bold_dyslexic, { SMALL_FONT_POINT_SIZE, LARGE_FONT_POINT_SIZE }))
    {
        LOG_ERROR(ERR_MAX, "Failed to load OpenDyslexic bold font!");
        return false;
    }
    if (!load_font_resource("fonts/opendyslexic-mono.ttf", resource_font_mono_dyslexic))
    {
        LOG_ERROR(ERR_MAX, "Failed to load OpenDyslexic mono font!");
        return false;
    }

    update_editor_font();

    LOG_DEBUG("Resources loaded!");
    return true;
}

FILDEF void free_editor_resources ()
{
    free_font         (resource_font_mono_dyslexic);
    free_font         (resource_font_bold_sans);
    free_font         (resource_font_bold_dyslexic);
    free_font         (resource_font_regular_libmono);
    free_font         (resource_font_regular_sans);
    free_font         (resource_font_regular_dyslexic);
    free_texture      (resource_icons);
    free_texture      (resource_checker_14);
    free_texture      (resource_checker_16);
    free_texture      (resource_checker_20);
    free_texture_atlas(resource_large);
    free_texture_atlas(resource_small);
}

FILDEF std::string build_resource_string (std::string str)
{
    return (is_path_absolute(str)) ? str : (resource_location + str);
}

FILDEF void update_editor_font ()
{
    current_editor_font = editor_settings.font_face;
}

FILDEF bool is_editor_font_opensans ()
{
    return (current_editor_font != "OpenDyslexic");
}

FILDEF Font& get_editor_regular_font ()
{
    return (is_editor_font_opensans()) ? resource_font_regular_sans : resource_font_regular_dyslexic;
}

FILDEF Font& get_editor_bold_font ()
{
    return (is_editor_font_opensans()) ? resource_font_bold_sans : resource_font_bold_dyslexic;
}

FILDEF Texture_Atlas& get_editor_atlas_large ()
{
    return resource_large;
}

FILDEF Texture_Atlas& get_editor_atlas_small ()
{
    return resource_small;
}
