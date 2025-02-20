GLOBAL constexpr const char* TILE_DATA_FILE = "data/editor_tiles.txt";
GLOBAL constexpr const char* FLIP_DATA_FILE = "data/editor_flips.txt";

GLOBAL constexpr float TILE_PANEL_ITEM_PAD   =    4;
GLOBAL constexpr float TILE_PANEL_ITEM_SIZE  =   20;
GLOBAL constexpr float TILE_PANEL_COLUMNS    =   11;
GLOBAL constexpr float TILE_PANEL_INNER_PAD  =    3;
GLOBAL constexpr float TILE_PANEL_LABEL_H    =   20;
GLOBAL constexpr float TILE_PANEL_INACTIVE_A = .33f;

typedef std::pair<Tile_ID, Tile_ID> Tile_Flip_Map;

struct Tile_Group
{
    std::string              name;
    std::string              desc;
    std::vector<Tile_ID>     tile;
    int            selected_index;
};

struct Tile_Panel
{
    std::vector<Tile_Flip_Map> flip_map_horz;
    std::vector<Tile_Flip_Map> flip_map_vert;

    std::map<Tile_Category, std::vector<Tile_Group>> category;

    quad bounds;

    int selected_category;
    int selected_group;

    bool mouse_down;

    float content_height;
    float scroll_offset;
};

GLOBAL Tile_Panel tile_panel;

FILDEF bool internal__is_category_active (Tile_Category category)
{
    if (!current_tab_is_level()) return false;
    const Tab& tab = get_current_tab();
    return tab.tile_layer_active[category_to_layer(category)];
}

FILDEF float internal__do_tile_panel_category (vec2& cursor, Tile_Category category_index, std::vector<Tile_Group>& category)
{
    // Determine how many rows of entities are present in the category.
    float items = CAST(float, category.size());
    float rows = ceilf(items / TILE_PANEL_COLUMNS);

    float total_item_pad = (rows - 1) * TILE_PANEL_ITEM_PAD;
    float category_pad = TILE_PANEL_INNER_PAD * 2;

    // Do a separate panel for the category's label before the main panel.
    float x = TILE_PANEL_INNER_PAD;
    float y = cursor.y;
    float w = (CONTROL_PANEL_WIDTH - category_pad);
    float h = TILE_PANEL_LABEL_H;

    bool is_active = internal__is_category_active(category_index);

    vec2 label_cur(0,0);
    begin_panel(x, y, w, h, (is_active) ? UI_NONE : UI_INACTIVE);

    set_panel_cursor(&label_cur);
    set_panel_cursor_dir(UI_DIR_DOWN);

    const char* label_name = NULL;
    switch (category_index)
    {
        case (TILE_CATEGORY_BASIC  ): label_name = "Basic";   break;
        case (TILE_CATEGORY_TAG    ): label_name = "Tag";     break;
        case (TILE_CATEGORY_OVERLAY): label_name = "Overlay"; break;
        case (TILE_CATEGORY_ENTITY ): label_name = "Entity";  break;
        case (TILE_CATEGORY_BACK1  ): label_name = "Back 1";  break;
        case (TILE_CATEGORY_BACK2  ): label_name = "Back 2";  break;
    }

    do_label(UI_ALIGN_LEFT,UI_ALIGN_CENTER, w, TILE_PANEL_LABEL_H, label_name);
    end_panel();

    cursor.y += TILE_PANEL_LABEL_H;

    // Bounding box for the particular category in the panel.
    x = TILE_PANEL_INNER_PAD;
    y = cursor.y;
    w = (CONTROL_PANEL_WIDTH - category_pad);
    h = (rows * TILE_PANEL_ITEM_SIZE) + category_pad + total_item_pad;

    vec2 cur(TILE_PANEL_INNER_PAD, TILE_PANEL_INNER_PAD);
    begin_panel(x, y, w, h, UI_NONE, ui_color_med_dark);

    set_draw_color(ui_color_ex_dark); // The outline/border for the category.
    draw_quad(0, get_panel_offset().y, w, get_panel_offset().y+h);

    Texture_Atlas& atlas = get_editor_atlas_small();

    set_tile_batch_texture(atlas.texture);
    set_tile_batch_color((is_active) ? vec4(1,1,1,1) : vec4(1,1,1,TILE_PANEL_INACTIVE_A));

    for (size_t i=0; i<category.size(); ++i)
    {
        vec2 tile_cursor = cur;
        tile_cursor.y += get_panel_offset().y;

        const Tile_Group& tile_group = category[i];
        quad tile_group_bounds = { tile_cursor.x, tile_cursor.y, TILE_PANEL_ITEM_SIZE, TILE_PANEL_ITEM_SIZE };

        if (is_active)
        {
            bool in_bounds = mouse_in_ui_bounds_xywh(tile_group_bounds);
            if (in_bounds)
            {
                push_status_bar_message("Tile Group: %s", tile_group.name.c_str());
                set_current_tooltip(tile_group.name, tile_group.desc);
            }
            if (tile_panel.mouse_down && !is_there_a_hit_ui_element())
            {
                if (in_bounds)
                {
                    tile_panel.selected_category = category_index;
                    tile_panel.selected_group = CAST(int, i);
                }
            }
        }

        if (current_tab_is_level() && !are_all_layers_inactive())
        {
            float qx = tile_cursor.x        - 1;
            float qy = tile_cursor.y        - 1;
            float qw = TILE_PANEL_ITEM_SIZE + 2;
            float qh = TILE_PANEL_ITEM_SIZE + 2;

            // If we are the currently selected group then we draw the highlight.
            if ((category_index == CAST(Tile_Category, tile_panel.selected_category)) && (CAST(int, i) == tile_panel.selected_group))
            {
                set_draw_color(ui_color_light);
                fill_quad(qx, qy, qx+qw, qy+qh);
            }
        }

        // We scissor the contents to avoid overspill.
        begin_scissor(tile_cursor.x, tile_cursor.y, TILE_PANEL_ITEM_SIZE, TILE_PANEL_ITEM_SIZE);

        float ex = tile_cursor.x + (TILE_PANEL_ITEM_SIZE/2);
        float ey = tile_cursor.y + (TILE_PANEL_ITEM_SIZE/2);

        Tile_ID selected_id = tile_group.tile[tile_group.selected_index];
        draw_batched_tile(ex, ey, &get_atlas_clip(atlas, selected_id));

        end_scissor();

        // Advance the cursor for the next entity's placement.
        float advance = TILE_PANEL_ITEM_SIZE + TILE_PANEL_ITEM_PAD;
        cur.x += advance;
        if (cur.x + TILE_PANEL_ITEM_SIZE > w)
        {
            cur.x = TILE_PANEL_INNER_PAD;
            cur.y += advance;
        }
    }

    flush_batched_tile();

    end_panel();

    // Set the cursor for the next category (if there is one).
    cursor.x  = TILE_PANEL_INNER_PAD;
    cursor.y += h;

    // Return the category height for use with the scrollbar.
    return h;
}

FILDEF void internal__load_flip_data (const GonObject& data, std::vector<Tile_Flip_Map>& flip)
{
    for (int i=0; i<CAST(int, data.children_array.size()); ++i)
    {
        Tile_ID a = CAST(Tile_ID, data[i][0].Int());
        Tile_ID b = CAST(Tile_ID, data[i][1].Int());

        flip.push_back(Tile_Flip_Map(a, b));
    }
}

FILDEF float internal__calculate_tile_panel_height ()
{
    float height = TILE_PANEL_INNER_PAD;
    for (auto& it: tile_panel.category)
    {
        const std::vector<Tile_Group>& c = it.second;

        float items          = CAST(float, c.size());
        float rows           = ceilf(items / TILE_PANEL_COLUMNS);
        float total_item_pad = (rows-1) * TILE_PANEL_ITEM_PAD;
        float category_pad   = (TILE_PANEL_INNER_PAD*2);

        height += (rows * TILE_PANEL_ITEM_SIZE) + category_pad + total_item_pad;
        height += TILE_PANEL_LABEL_H;
    }
    return height;
}

FILDEF void internal__set_category_as_active (Tile_Category category)
{
    int old_selected_category = tile_panel.selected_category;
    int old_selected_group = tile_panel.selected_group;

    tile_panel.selected_category = category;
    tile_panel.selected_group = 0;

    // Make sure we are actually jumping to an active category.
    if (!internal__is_category_active(CAST(Tile_Category, tile_panel.selected_category)))
    {
        select_next_active_group();
    }

    if (old_selected_category != tile_panel.selected_category || old_selected_group != tile_panel.selected_group)
    {
        // When the selected gets changed then we make a new state.
        if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
        {
            if (level_editor.tool_state != Tool_State::IDLE)
            {
                new_level_history_state(Level_History_Action::NORMAL);
            }
        }
    }
}

FILDEF void internal__jump_to_category (Tile_Category category)
{
    if (current_tab_is_level())
    {
        if (!are_all_layers_inactive())
        {
            if (is_window_focused("Main"))
            {
                internal__set_category_as_active(category);
            }
        }
    }
}

FILDEF bool init_tile_panel ()
{
    tile_panel.flip_map_horz.clear();
    tile_panel.flip_map_vert.clear();

    tile_panel.category.clear();

    try
    {
        // Load all of the tile groups and sort them into their categories.
        GonObject tile_gon_data = GonObject::LoadFromBuffer(load_string_resource(TILE_DATA_FILE))["tiles"];

        for (auto& it: tile_gon_data.children_map)
        {
            const GonObject& category_data = tile_gon_data.children_array[it.second];
            if (std::stoi(it.first) < TILE_CATEGORY_TOTAL)
            {
                Tile_Category category_id = CAST(Tile_Category, std::stoi(it.first));
                for (auto& tile_group_gon_data: category_data.children_array)
                {
                    Tile_Group group;
                    group.selected_index = 0;
                    group.name = tile_group_gon_data["name"].String();
                    group.desc = tile_group_gon_data["tooltip"].String();
                    for (int i=0; i<CAST(int, tile_group_gon_data["id"].size()); ++i)
                    {
                        Tile_ID tile_id = CAST(Tile_ID, tile_group_gon_data["id"][i].Int());
                        group.tile.push_back(tile_id);
                    }
                    tile_panel.category[category_id].push_back(group);
                }
            }
        }

        // Load flip mappings between the tile IDs for smart level flipping.
        GonObject flip_gon_data = GonObject::LoadFromBuffer(load_string_resource(FLIP_DATA_FILE))["flip"];

        internal__load_flip_data(flip_gon_data["horz"], tile_panel.flip_map_horz);
        internal__load_flip_data(flip_gon_data["vert"], tile_panel.flip_map_vert);
    }
    catch (const char* msg)
    {
        LOG_ERROR(ERR_MED, "%s", msg);
        return false;
    }

    // The starting selected is always the first entry in the first category.
    tile_panel.selected_category = TILE_CATEGORY_BASIC;
    tile_panel.selected_group = 0;

    // We need to calculate the height of all the tiles panel content so that
    // we can use this height value during the panel do_scrollbar operation.
    tile_panel.content_height = internal__calculate_tile_panel_height();
    tile_panel.scroll_offset = 0;

    return true;
}

FILDEF void do_tile_panel (bool scrollbar)
{
    set_ui_font(&get_editor_regular_font());

    vec2 cursor(TILE_PANEL_INNER_PAD, 0);
    tile_panel.bounds = { 0, 0, get_panel_w(), get_panel_h() };
    if (is_layer_panel_present())
    {
        tile_panel.bounds.h = roundf(tile_panel.bounds.h - get_layer_panel_height());
    }

    begin_panel(tile_panel.bounds, UI_NONE, ui_color_medium);

    if (scrollbar)
    {
        float x = CONTROL_PANEL_WIDTH;
        float y = CONTROL_PANEL_INNER_PAD;
        float w = CONTROL_PANEL_SCROLLBAR_WIDTH - CONTROL_PANEL_INNER_PAD;
        float h = get_viewport().h - (CONTROL_PANEL_INNER_PAD * 2);

        quad sb = { x,y,w,h };

        do_scrollbar(sb, tile_panel.content_height, tile_panel.scroll_offset);
    }

    for (auto& it: tile_panel.category)
    {
        internal__do_tile_panel_category(cursor, it.first, it.second);
    }

    end_panel();
}

FILDEF bool tile_panel_needs_scrollbar ()
{
    return (tile_panel.content_height > tile_panel.bounds.h);
}

FILDEF void handle_tile_panel_events ()
{
    if (current_tab_is_level())
    {
        if (!are_all_layers_inactive())
        {
            if (is_window_focused("Main"))
            {
                switch (main_event.type)
                {
                    case (SDL_MOUSEBUTTONDOWN):
                    case (SDL_MOUSEBUTTONUP):
                    {
                        if (main_event.button.button == SDL_BUTTON_LEFT)
                        {
                            tile_panel.mouse_down = (main_event.button.state == SDL_PRESSED);
                        }
                    } break;
                    case (SDL_MOUSEWHEEL):
                    {
                        if (!mouse_is_over_tab_bar())
                        {
                            if (is_key_mod_state_active(0))
                            {
                                if      (main_event.wheel.y > 0) increment_selected_tile();
                                else if (main_event.wheel.y < 0) decrement_selected_tile();
                            }
                            else if (is_key_mod_state_active(KMOD_ALT))
                            {
                                if      (main_event.wheel.y > 0) increment_selected_group();
                                else if (main_event.wheel.y < 0) decrement_selected_group();
                            }
                            else if (is_key_mod_state_active(KMOD_ALT|KMOD_SHIFT))
                            {
                                if      (main_event.wheel.y > 0) increment_selected_category();
                                else if (main_event.wheel.y < 0) decrement_selected_category();
                            }
                        }
                    } break;
                }
            }
        }
    }
}

FILDEF float get_tile_panel_height ()
{
    return (tile_panel.bounds.h + 1);
}

FILDEF void reload_tile_graphics ()
{
    free_texture_atlas(resource_large);
    free_texture_atlas(resource_small);

    if (editor_settings.tile_graphics == "new")
    {
        load_atlas_resource("textures/editor_icons/new_large.txt", resource_large);
        load_atlas_resource("textures/editor_icons/new_small.txt", resource_small);
    }
    else
    {
        load_atlas_resource("textures/editor_icons/old_large.txt", resource_large);
        load_atlas_resource("textures/editor_icons/old_small.txt", resource_small);
    }
}

FILDEF Tile_Category get_selected_category ()
{
    return CAST(Tile_Category, tile_panel.selected_category);
}

FILDEF Tile_ID get_selected_tile ()
{
    const auto& category = tile_panel.category[tile_panel.selected_category];
    const auto& group = category[tile_panel.selected_group];
    return group.tile[group.selected_index];
}

FILDEF Level_Layer get_selected_layer ()
{
    return category_to_layer(tile_panel.selected_category);
}

FILDEF Level_Layer category_to_layer (Tile_Category category)
{
    switch (category)
    {
        case (TILE_CATEGORY_BASIC  ): return LEVEL_LAYER_ACTIVE;  break;
        case (TILE_CATEGORY_TAG    ): return LEVEL_LAYER_TAG;     break;
        case (TILE_CATEGORY_OVERLAY): return LEVEL_LAYER_OVERLAY; break;
        case (TILE_CATEGORY_ENTITY ): return LEVEL_LAYER_ACTIVE;  break;
        case (TILE_CATEGORY_BACK1  ): return LEVEL_LAYER_BACK1;   break;
        case (TILE_CATEGORY_BACK2  ): return LEVEL_LAYER_BACK2;   break;
    }
    return LEVEL_LAYER_ACTIVE;
}

STDDEF void select_next_active_group ()
{
    if (!are_all_layers_inactive())
    {
        while (!internal__is_category_active(tile_panel.selected_category))
        {
            ++tile_panel.selected_category;
            if (tile_panel.selected_category > TILE_CATEGORY_BACK2)
            {
                tile_panel.selected_category = TILE_CATEGORY_BASIC;
            }
            tile_panel.selected_group = 0;
        }
    }
}

STDDEF void select_prev_active_group ()
{
    if (!are_all_layers_inactive())
    {
        while (!internal__is_category_active(tile_panel.selected_category))
        {
            --tile_panel.selected_category;
            if (tile_panel.selected_category < TILE_CATEGORY_BASIC)
            {
                tile_panel.selected_category = TILE_CATEGORY_BACK2;
            }
            tile_panel.selected_group = 0;
        }
    }
}

FILDEF void reset_selected_group ()
{
    tile_panel.selected_category = 0;
    select_next_active_group();
}

FILDEF void increment_selected_tile ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& group = tile_panel.category[tile_panel.selected_category][tile_panel.selected_group];
        int old_selected_index = group.selected_index;
        if ((++group.selected_index) > CAST(int, group.tile.size()-1))
        {
            group.selected_index = 0;
        }
        // Feels correct that a new history state should be made.
        if (old_selected_index != group.selected_index)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

FILDEF void decrement_selected_tile ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& group = tile_panel.category[tile_panel.selected_category][tile_panel.selected_group];
        int old_selected_index = group.selected_index;
        if ((--group.selected_index) < 0)
        {
            group.selected_index = CAST(int, group.tile.size()-1);
        }
        // Feels correct that a new history state should be made.
        if (old_selected_index != group.selected_index)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

FILDEF void increment_selected_group ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& category = tile_panel.category[tile_panel.selected_category];
        int old_selected_group = tile_panel.selected_group;
        if ((++tile_panel.selected_group) > CAST(int, category.size()-1))
        {
            tile_panel.selected_group = 0;
        }
        // Feels correct that a new history state should be made.
        if (old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

FILDEF void decrement_selected_group ()
{
    if (!current_tab_is_level()) return;

    if (!are_all_layers_inactive())
    {
        auto& category = tile_panel.category[tile_panel.selected_category];
        int old_selected_group = tile_panel.selected_group;
        if ((--tile_panel.selected_group) < 0)
        {
            tile_panel.selected_group = CAST(int, category.size()-1);
        }
        // Feels correct that a new history state should be made.
        if (old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

FILDEF void increment_selected_category ()
{
    if (!current_tab_is_level()) return;

    int old_selected_category = tile_panel.selected_category;
    int old_selected_group = tile_panel.selected_group;

    if (!are_all_layers_inactive())
    {
        if ((++tile_panel.selected_category) > CAST(int, TILE_CATEGORY_BACK2))
        {
            tile_panel.selected_category = TILE_CATEGORY_BASIC;
        }
        tile_panel.selected_group = 0;
        select_next_active_group();
        // Feels correct that a new history state should be made.
        if (old_selected_category != tile_panel.selected_category || old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

FILDEF void decrement_selected_category ()
{
    if (!current_tab_is_level()) return;

    int old_selected_category = tile_panel.selected_category;
    int old_selected_group = tile_panel.selected_group;

    if (!are_all_layers_inactive())
    {
        if ((--tile_panel.selected_category) < CAST(int, TILE_CATEGORY_BASIC))
        {
            tile_panel.selected_category = TILE_CATEGORY_BACK2;
        }
        tile_panel.selected_group = 0;
        select_prev_active_group();
        // Feels correct that a new history state should be made.
        if (old_selected_category != tile_panel.selected_category || old_selected_group != tile_panel.selected_group)
        {
            if (level_editor.tool_type == Tool_Type::BRUSH || level_editor.tool_type == Tool_Type::FILL)
            {
                if (level_editor.tool_state != Tool_State::IDLE)
                {
                    new_level_history_state(Level_History_Action::NORMAL);
                }
            }
        }
    }
}

FILDEF Tile_ID get_tile_horizontal_flip (Tile_ID id)
{
    if (id)
    {
        for (const auto& pair: tile_panel.flip_map_horz)
        {
            if (id == pair.first)  return pair.second;
            if (id == pair.second) return pair.first;
        }
    }
    return id;
}
FILDEF Tile_ID get_tile_vertical_flip (Tile_ID id)
{
    if (id)
    {
        for (const auto& pair: tile_panel.flip_map_vert)
        {
            if (id == pair.first)  return pair.second;
            if (id == pair.second) return pair.first;
        }
    }
    return id;
}

FILDEF void jump_to_category_basic ()
{
    internal__jump_to_category(TILE_CATEGORY_BASIC);
}

FILDEF void jump_to_category_tag ()
{
    internal__jump_to_category(TILE_CATEGORY_TAG);
}

FILDEF void jump_to_category_overlay ()
{
    internal__jump_to_category(TILE_CATEGORY_OVERLAY);
}

FILDEF void jump_to_category_entity ()
{
    internal__jump_to_category(TILE_CATEGORY_ENTITY);
}

FILDEF void jump_to_category_back1 ()
{
    internal__jump_to_category(TILE_CATEGORY_BACK1);
}

FILDEF void jump_to_category_back2 ()
{
    internal__jump_to_category(TILE_CATEGORY_BACK2);
}
