#pragma once

// Various clips for the different icons that are found in resources_icons.

GLOBAL constexpr quad CLIP_NONE        = {   0,   0,  0,  0 };
GLOBAL constexpr quad CLIP_BULLET      = {  96, 108, 24, 12 };
GLOBAL constexpr quad CLIP_BRUSH       = {   0,   0, 24, 24 };
GLOBAL constexpr quad CLIP_FILL        = {  24,   0, 24, 24 };
GLOBAL constexpr quad CLIP_SELECT      = {  48,   0, 24, 24 };
GLOBAL constexpr quad CLIP_GRID        = {  72,   0, 24, 24 };
GLOBAL constexpr quad CLIP_BOUNDS      = {   0,  24, 24, 24 };
GLOBAL constexpr quad CLIP_LAYERS      = {  24,  24, 24, 24 };
GLOBAL constexpr quad CLIP_CAMERA      = {  48,  24, 24, 24 };
GLOBAL constexpr quad CLIP_FLIP_H      = {   0,  48, 24, 24 };
GLOBAL constexpr quad CLIP_FLIP_V      = {  24,  48, 24, 24 };
GLOBAL constexpr quad CLIP_MIRROR_H    = {  48,  48, 24, 24 };
GLOBAL constexpr quad CLIP_MIRROR_V    = {  72,  48, 24, 24 };
GLOBAL constexpr quad CLIP_CUT         = {   0,  72, 24, 24 };
GLOBAL constexpr quad CLIP_COPY        = {  24,  72, 24, 24 };
GLOBAL constexpr quad CLIP_DESELECT    = {  48,  72, 24, 24 };
GLOBAL constexpr quad CLIP_CLEAR       = {  72,  72, 24, 24 };
GLOBAL constexpr quad CLIP_ARROW_UP    = {  48,  96, 12, 12 };
GLOBAL constexpr quad CLIP_ARROW_RIGHT = {  60,  96, 12, 12 };
GLOBAL constexpr quad CLIP_ARROW_LEFT  = {  48, 108, 12, 12 };
GLOBAL constexpr quad CLIP_ARROW_DOWN  = {  60, 108, 12, 12 };
GLOBAL constexpr quad CLIP_CROSS       = {  72,  96, 24, 24 };
GLOBAL constexpr quad CLIP_EYE         = {  96,  96, 24, 12 };
GLOBAL constexpr quad CLIP_RESIZE_N    = {  24,  96, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE_E    = {  36,  96, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE_S    = {  36, 108, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE_W    = {  24, 108, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE_NE   = {  12,  96, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE_SE   = {  12, 108, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE_SW   = {   0, 108, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE_NW   = {   0,  96, 12, 12 };
GLOBAL constexpr quad CLIP_RESIZE      = {  72,  24, 24, 24 };
GLOBAL constexpr quad CLIP_PACK        = {  96,  24, 24, 24 };
GLOBAL constexpr quad CLIP_UNPACK      = {  96,  48, 24, 24 };
GLOBAL constexpr quad CLIP_GUIDES      = {  96,   0, 24, 24 };
GLOBAL constexpr quad CLIP_ENTITY      = {  96,  72, 24, 24 };
GLOBAL constexpr quad CLIP_NEW         = { 120,   0, 24, 24 };
GLOBAL constexpr quad CLIP_LOAD        = { 144,   0, 24, 24 };
GLOBAL constexpr quad CLIP_SAVE        = { 168,   0, 24, 24 };
GLOBAL constexpr quad CLIP_SAVE_AS     = { 192,   0, 24, 24 };
GLOBAL constexpr quad CLIP_UNDO        = { 216,   0, 24, 24 };
GLOBAL constexpr quad CLIP_REDO        = { 120,  24, 24, 24 };
GLOBAL constexpr quad CLIP_ZOOM_OUT    = { 144,  24, 24, 24 };
GLOBAL constexpr quad CLIP_ZOOM_IN     = { 168,  24, 24, 24 };
GLOBAL constexpr quad CLIP_RUN         = { 192,  24, 24, 24 };
GLOBAL constexpr quad CLIP_SETTINGS    = { 216,  24, 24, 24 };
GLOBAL constexpr quad CLIP_ABOUT       = { 120,  48, 24, 24 };
GLOBAL constexpr quad CLIP_HELP        = { 144,  48, 24, 24 };
GLOBAL constexpr quad CLIP_UPDATE      = { 168,  48, 24, 24 };
GLOBAL constexpr quad CLIP_BUG         = { 192,  48, 24, 24 };
GLOBAL constexpr quad CLIP_BACKUPS     = { 144,   0, 24, 24 };

// Globally accessible and generic resources that can be used by the editor.
// We also have a global copy of the no tile/spawn image texture so that we
// can use it in the level editor in case a level is loaded that has IDs we
// no longer recognise -- just so that the user doesn't think they're empty.

GLOBAL Texture       resource_icons;
GLOBAL Font          resource_font_regular_sans;
GLOBAL Font          resource_font_regular_dyslexic;
GLOBAL Font          resource_font_regular_libmono;
GLOBAL Font          resource_font_mono_dyslexic;
GLOBAL Font          resource_font_bold_sans;
GLOBAL Font          resource_font_bold_dyslexic;
GLOBAL Texture       resource_checker_14;
GLOBAL Texture       resource_checker_16;
GLOBAL Texture       resource_checker_20;
GLOBAL Texture_Atlas resource_large;
GLOBAL Texture_Atlas resource_small;

FILDEF bool init_resource_manager ();
FILDEF void get_resource_location ();

FILDEF bool            load_texture_resource (std::string file_name, Texture& tex, Texture_Wrap wrap = TEXTURE_WRAP_CLAMP_TO_EDGE);
FILDEF bool            load_atlas_resource   (std::string file_name, Texture_Atlas& atlas);
FILDEF bool            load_font_resource    (std::string file_name, Font& fnt, std::vector<int> pt = { DEFAULT_FONT_GLYPH_POINT_SIZE }, float csz = DEFAULT_FONT_GLYPH_CACHE_SIZE);
FILDEF Shader          load_shader_resource  (std::string file_name);
FILDEF std::vector<u8> load_binary_resource  (std::string file_name);
FILDEF SDL_Surface*    load_surface_resource (std::string file_name);
FILDEF std::string     load_string_resource  (std::string file_name);

FILDEF bool load_editor_resources ();
FILDEF void free_editor_resources ();

FILDEF std::string build_resource_string (std::string str);

FILDEF void update_editor_font      ();
FILDEF bool is_editor_font_opensans ();

FILDEF Font& get_editor_regular_font ();
FILDEF Font& get_editor_bold_font    ();

FILDEF Texture_Atlas& get_editor_atlas_large ();
FILDEF Texture_Atlas& get_editor_atlas_small ();
