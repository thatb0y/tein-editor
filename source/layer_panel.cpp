GLOBAL constexpr const char* TILE_LAYER_INFO  = "Toggle this tile layer's visibility (invisible layers can't be interacted with).";

GLOBAL const vec4 LAYER_COLORS[LEVEL_LAYER_TOTAL]
{
{ .75f, .38f, .38f, 1 }, // Tag
{ .44f, .51f, .72f, 1 }, // Overlay
{ .79f, .69f, .20f, 1 }, // Active
{ .45f, .59f, .41f, 1 }, // Back 1
{ .57f, .48f, .71f, 1 }  // Back 2
};

GLOBAL constexpr float LAYER_PANEL_INNER_PAD =  3;
GLOBAL constexpr float LAYER_PANEL_BUTTON_H  = 24;

GLOBAL float layer_panel_content_height;
GLOBAL float layer_panel_panel_height;

GLOBAL float layer_panel_scroll_offset;

GLOBAL quad layer_panel_bounds;

FILDEF bool internal__do_layer_button (UI_Flag flags, int layer, const char* name, const char* info)
{
    const quad& clip = (flags & UI_INACTIVE) ? CLIP_CROSS : CLIP_EYE;

    constexpr float PAD = 5;
    vec2 cursor(PAD, 0);

    float bw = get_panel_w();
    float bh = LAYER_PANEL_BUTTON_H;

    // If not inactive then we need to determine if this is the active layer.
    if (!(flags & UI_INACTIVE))
    {
        // If the tool is the select tool then technically all of the layers
        // are active (except for disabled ones). So it makes sense to just
        // highlight ever single layer when using this particular tool.
        if (level_editor.tool_type == Tool_Type::SELECT)
        {
            flags |= UI_HIGHLIGHT;
        }
        else
        {
            Tile_Category category = get_selected_category();
            if (category_to_layer(category) == CAST(Level_Layer, layer))
            {
                flags |= UI_HIGHLIGHT;
            }
        }
    }

    bool result = begin_click_panel(NULL, bw,bh, flags, info);

    set_panel_cursor(&cursor);
    set_panel_cursor_dir(UI_DIR_RIGHT);

    float w = 10;
    float h = (LAYER_PANEL_BUTTON_H-4)-1; // -1 due to separator!

    cursor.y = (bh-h)/2;
    do_quad(w, h, LAYER_COLORS[layer]);
    cursor.y = 0;
    advance_panel_cursor(PAD);
    do_icon(24, get_panel_h(), resource_icons, &clip);
    advance_panel_cursor(PAD);
    do_label(UI_ALIGN_LEFT, UI_ALIGN_CENTER, get_panel_h(), name);

    end_panel();
    return result;
}

FILDEF void internal__toggle_layer (Level_Layer layer)
{
    if (current_tab_is_level())
    {
        Tab& tab = get_current_tab();
        tab.tile_layer_active[layer] = !tab.tile_layer_active[layer];
        select_next_active_group();
    }
}

FILDEF void internal__toggle_layer_action (Level_Layer layer)
{
    if (current_tab_is_level())
    {
        if (is_window_focused("Main"))
        {
            bool all_layers_were_inactive = are_all_layers_inactive();
            internal__toggle_layer(layer);
            // If we're coming from all layers being inactive we need to find an entity
            // we can select now that there are entities that can be selected again.
            if (all_layers_were_inactive && !are_all_layers_inactive())
            {
                reset_selected_group();
            }
        }
    }
}

FILDEF void init_layer_panel ()
{
    layer_panel_scroll_offset   = 0;
    layer_panel_bounds          = {};
    // Calculate the content height for the layer panel.
    layer_panel_content_height  = LAYER_PANEL_BUTTON_H * LEVEL_LAYER_TOTAL;
    layer_panel_content_height -= 1; // We don't care about the last separator!
    layer_panel_panel_height    = 0;
}

FILDEF void do_layer_panel (bool scrollbar)
{
    if (!is_layer_panel_present()) return;

    layer_panel_bounds.x = 0;
    layer_panel_bounds.y = get_tile_panel_height();
    layer_panel_bounds.w = get_panel_w();
    layer_panel_bounds.h = get_panel_h() - get_tile_panel_height();

    vec2 cursor(0,0);

    set_ui_texture(&resource_icons);
    set_ui_font(&get_editor_regular_font());

    constexpr float PAD = LAYER_PANEL_INNER_PAD;

    begin_panel(layer_panel_bounds, UI_NONE, ui_color_medium);
    begin_panel(PAD, PAD, CONTROL_PANEL_WIDTH-(PAD*2), layer_panel_bounds.h-(PAD*2), UI_NONE, ui_color_med_dark);

    // NOTE: We do this to add a 1px border around the actual layer buttons in
    // the case that the panel is too small and needs a scrollbar it looks
    // nicer. Its a bit hacky and at some point we should probs have a feature
    // to add a padding border around a panel but for now we just do this...
    begin_panel(1, 1, get_panel_w()-2, get_panel_h()-2, UI_NONE, ui_color_med_dark);
    layer_panel_panel_height = get_panel_h();

    set_panel_cursor(&cursor);
    set_panel_cursor_dir(UI_DIR_DOWN);

    if (scrollbar)
    {
        float sx =  1 + get_viewport().w + CONTROL_PANEL_INNER_PAD;
        float sy = -1;
        float sw =  CONTROL_PANEL_SCROLLBAR_WIDTH - CONTROL_PANEL_INNER_PAD;
        float sh =  2 + get_viewport().h;

        do_scrollbar(sx,sy,sw,sh, layer_panel_content_height, layer_panel_scroll_offset);
    }

    bool all_layers_were_inactive = are_all_layers_inactive();
    Tab& tab = get_current_tab();

    for (Level_Layer i=LEVEL_LAYER_TAG; i<LEVEL_LAYER_TOTAL; ++i)
    {
        const char* layer_name = NULL;
        switch (i)
        {
            case (LEVEL_LAYER_TAG    ): layer_name = "Tag";     break;
            case (LEVEL_LAYER_OVERLAY): layer_name = "Overlay"; break;
            case (LEVEL_LAYER_ACTIVE ): layer_name = "Active";  break;
            case (LEVEL_LAYER_BACK1  ): layer_name = "Back 1";  break;
            case (LEVEL_LAYER_BACK2  ): layer_name = "Back 2";  break;
        }

        UI_Flag tile_flag = (tab.tile_layer_active[i]) ? UI_NONE : UI_INACTIVE;
        if (internal__do_layer_button(tile_flag, i, layer_name, TILE_LAYER_INFO))
        {
            internal__toggle_layer(i);
        }
    }

    // If we're coming from all layers being inactive we need to find an entity
    // we can select now that there are entities that can be selected again.
    if (all_layers_were_inactive && !are_all_layers_inactive())
    {
        reset_selected_group();
    }

    end_panel();

    end_panel();
    end_panel();
}

FILDEF float get_layer_panel_height ()
{
    return ceilf((layer_panel_content_height + 1 + 2 + (LAYER_PANEL_INNER_PAD * 2)));
}

FILDEF bool layer_panel_needs_scrollbar ()
{
    return ((current_tab_is_level()) ? (layer_panel_content_height > layer_panel_panel_height) : false);
}

FILDEF bool is_layer_panel_present ()
{
    return current_tab_is_level();
}

FILDEF void toggle_layer_active ()
{
    internal__toggle_layer_action(LEVEL_LAYER_ACTIVE);
}

FILDEF void toggle_layer_tag ()
{
    internal__toggle_layer_action(LEVEL_LAYER_TAG);
}

FILDEF void toggle_layer_overlay ()
{
    internal__toggle_layer_action(LEVEL_LAYER_OVERLAY);
}

FILDEF void toggle_layer_back1 ()
{
    internal__toggle_layer_action(LEVEL_LAYER_BACK1);
}

FILDEF void toggle_layer_back2 ()
{
    internal__toggle_layer_action(LEVEL_LAYER_BACK2);
}
