#include <pebble.h>
#include "animator.h"

typedef struct _tile_menu_ TileMenu;

TileMenu *      tile_menu_create(GRect frame, Window * window, unsigned tiles, unsigned tiles_per_view, unsigned tiles_per_row);
void            tile_menu_destroy(TileMenu * menu);

void            tile_menu_draw(TileMenu * menu);

Layer *         tile_menu_get_next(TileMenu * menu);
Layer *         tile_menu_get_prev(TileMenu * menu);
Layer *         tile_menu_get_curr(TileMenu * menu);
bool            tile_menu_at_end(TileMenu * menu);
bool            tile_menu_at_begin(TileMenu * menu);

GRect           tile_menu_get_bounds(TileMenu * menu);
int             tile_menu_get_tile_count(TileMenu * menu);
Window *        tile_menu_get_window(TileMenu * menu);
Layer *         tile_menu_get_layer(TileMenu * menu);

Layer *         tile_menu_get_selected(TileMenu * menu);
void            tile_menu_set_selected_next(TileMenu * menu);
void            tile_menu_set_selected_prev(TileMenu * menu);
