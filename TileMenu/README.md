# Pebble TileMenu

A tiled style menu which uses generic ```Layer``` objects bound by a Menu window that spans horizontally and vertically. 
TileMenu uses a ```ScrollLayer``` base Layer to allow for scrollable functionality like Pebble's ```SimpleMenuLayer``` and ```MenuLayer```.
TileMenu manages all the memory creation and deletion within it's bounds and can be destroyed simply by calling the ```tile_menu_destroy``` method.

TileMenu also implements a custom Selector which changes direction based on the start and end bounds of TileMenu. Custom key operations
can be programmed for the TileMenu but the standard implementation would be the following:

```c
// main.c

// Click Configurator to assign callbacks
void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

// UP Button Config to move the selector to the relative previous tile
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    tile_menu_set_selected_prev(menu);
}
 
// DOWN Button Config to move the selector to the relative next tile
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    tile_menu_set_selected_next(menu);
}

void select_click_handler(ClickRecognizerRef recognizer, void *context) {
// ...
}
```

A basic implementation of TileMenu would be the following:

```c
// main.c
#include <pebble.h>
#include "tile_menu.h"

Window * window;	
static TileMenu * menu;


void add_text_to_tile(Layer * layer, GContext * ctx) {
  if(!layer || !ctx)
    return;
  
  // Loads all the tiles with "test" content
  graphics_draw_text(ctx, 
                     "test", 
                     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                     layer_get_bounds(layer),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter,
                     NULL);
  graphics_context_set_fill_color(ctx, GColorClear);
  graphics_context_set_text_color(ctx, GColorWhite);
}

void window_load(Window * window) {
  if(!window)
      return;

  // Creates a TileMenu within the main window which will fill the entire Pebble screen
  // With 9 maximum tiles @ 3x3
  menu = tile_menu_create(layer_get_bounds(window_get_root_layer(window)), window, 9, 3, 3);

  // Iterates through all the tiles and sets the update callback since they
  // each tile is a basic Layer object
  for(Layer * layer = tile_menu_get_curr(menu); 
      !tile_menu_at_end(menu); 
      layer = tile_menu_get_next(menu)) {
    if(layer) {
        layer_set_update_proc(layer,add_text_to_tile);
    }
  }

  // Draws all the Layers inside the TileMenu
  // Important to set the update callbacks before doing this!
  tile_menu_draw(menu);
  // Adds the TileMenu to the stack
  layer_add_child(window_get_root_layer(window), tile_menu_get_layer(menu));
}

void window_unload(Window * window) {
  tile_menu_destroy(menu);
}

void init(void) {  
	window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
  });

  window_set_background_color(window,GColorBlack);
  window_set_fullscreen(window,true);
  window_set_click_config_provider(window, click_config_provider);
	window_stack_push(window, true);
}

void deinit(void) {
	window_destroy(window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}

```

