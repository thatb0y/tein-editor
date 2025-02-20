#pragma once

typedef u32 Editor_Event;

GLOBAL constexpr u32 EDITOR_EVENT_UI_REDRAW     = 0;
GLOBAL constexpr u32 EDITOR_EVENT_BACKUP_TAB    = 1;
GLOBAL constexpr u32 EDITOR_EVENT_HOTLOAD       = 2;
GLOBAL constexpr u32 EDITOR_EVENT_COOLDOWN      = 3;
GLOBAL constexpr u32 EDITOR_EVENT_BLINK_CURSOR  = 4;
GLOBAL constexpr u32 EDITOR_EVENT_GPAK_PROGRESS = 5;
GLOBAL constexpr u32 EDITOR_EVENT_SHOW_TOOLTIP  = 6;
GLOBAL constexpr u32 EDITOR_EVENT_SHOW_UPDATE   = 7;
GLOBAL constexpr u32 EDITOR_EVENT_ARROW_PAN     = 8;

FILDEF void push_editor_event (Editor_Event id,
                               void* data1,
                               void* data2);
