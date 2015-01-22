/** TileMenu Layer
 *     Written By: Mark Zammit
 */
#include <pebble.h>
#include "animator.h"

typedef struct _tile_menu_ TileMenu;
typedef void (*TileMenuCallback)(TileMenu * menu, void * context);

/**    TileMenu Callbacks
 *    @brief: All the callbacks that the TileMenu exposes for use by applications.
 *            
 *    @click_config_provider      Provider function to set up the SELECT button handlers. 
 *                                This will be called after the TileMenu layer has configured 
 *                                the click configurations for the up/down buttons, so it can 
 *                                also be used to modify the default up/down behavior.
 *
 *    @content_changed_handler    Called every time the content on-screen changes, this occurs
 *                                when a scroll event has been resolved and tiles have been
 *                                reloaded.
 */
typedef struct _tile_menu_callbacks_ {
    ClickConfigProvider click_config_provider;
    TileMenuCallback content_changed_handler;
} TileMenuCallbacks;


/**   Create Method 
 *    @brief: Creates a new TileMenu layer on the haep and initializes it with default values
 *
 *    @frame        Default frame size of TileMenu
 *    @window       Window layer that TileMenu will be attached to
 *    @tiles        Maximum number of tiles to populate the TileMenu with
 *    @tiles_per_view    Number of tiles displayed vertically, i.e. rows
 *    @tiles_per_row     Number of tiles displayed horizontally, i.e. columns
 *
 *    @returns: Newly created and initialised TileMenu to be attached to a window
 *
 *    N.B. TileMenu is NOT automatically attached to a window since base Layer
 *         objects are used and therefore need to be initialised by the application
 *         before they are drawn by the TileMenu and hence attached to a window.
 */
TileMenu *      tile_menu_create(GRect frame, Window * window, unsigned tiles, unsigned tiles_per_view, unsigned tiles_per_row);
/**    Destroy Method
 *    @brief: Destroys the TileMenu and all of its tiles and any other objects creatd on the heap
 */
void            tile_menu_destroy(TileMenu * menu);

/**    Draw Method
 *    @brief: This will add all the tiles to the underlying TileMenu layer.
 *            Make sure to create the content of all the tiles before calling tis method!
 */
void            tile_menu_draw(TileMenu * menu);

/**    Callback Context Override
 *    @brief: Sets a new callback context, this context is passed into TileMenuCallbacks.
 *        
 *    N.B.This only comes into effect if tile_menu_set_callbacks() is used.
 */
void            tile_menu_set_context(TileMenu * menu, void * context);
/**    Callback Overrides
 *    @brief: Sets the callbacks that the TileMenu exposes. The @context as set by
 *            tile_menu_set_context() is passed into each of the callbacks.
 *            This MUST be used AFTER the TileMenu has been attached to a window or
 *            it will have no effect on SELECT events and will use the default 
 *            ClickHandler.
 *
 *    N.B. See TileMenuCallbacks declaration for the different callbacks.
 */
void            tile_menu_set_callbacks(TileMenu * menu, TileMenuCallbacks callbacks);

/**    Get Next Tile Layer
 *    @brief: Returns the NEXT tile Layer in the menu if any.
 *    @returns: Layer of the NEXT tile or NULL if non available.
 *
 *    N.B. Calling this method after reaching the END tile will reset it to the start.
 */
Layer *         tile_menu_get_next(TileMenu * menu);
/**    Get Current Tile Layer
 *    @brief: Returns the CURRENT tile Layer in the menu if any.
 *    @returns: Layer of the CURRENT tile or NULL if non available.
 */
Layer *         tile_menu_get_prev(TileMenu * menu);
/**    Get Previous Tile Layer
 *    @brief: Returns the PREVIOUS tile Layer in the menu if any.
 *    @returns: Layer of the PREVIOUS tile or NULL if non available.
 *
 *    N.B. Calling this method after reaching the START tile will reset it to the end.
 */
Layer *         tile_menu_get_curr(TileMenu * menu);

/**    Tile Iteration End Test
 *    @brief: Tests whether iterating through the TileMenu has reached the END tile.
 *    @returns: @true on reaching the END tile, @false otherwise.
 */
bool            tile_menu_at_end(TileMenu * menu);
/**    Tile Iteration Beginning Test
 *    @brief: Tests whether iterating through the TileMenu has reached the START tile.
 *    @returns: @true on reaching the START tile, @false otherwise.
 */
bool            tile_menu_at_begin(TileMenu * menu);

/**    Get TileMenu Bounds
 *    @brief: Gets the visible bounds of the TileMenu.
 *    @returns: GRect containing TileMenu visible frame, GRectZero if uninitialised TileMenu.
 */
GRect           tile_menu_get_bounds(TileMenu * menu);
/**    Get Tile Count
 *    @brief: Gets a count of the total number of tiles in the TileMenu.
 *    @returns: Returns the tile count as > 0, -1 if uninitialised TileMenu.
 */
int             tile_menu_get_tile_count(TileMenu * menu);
/**    Get TileMenu Parent Window
 *    @brief: Gets the Window that TileMenu is attached to.
 *    @returns: Returns the parent Window, NULL if uninitialised TileMenu.
 */
Window *        tile_menu_get_window(TileMenu * menu);
/**    Get TileMenu Layer
 *    @brief: Gets the Layer object of the TileMenu, since TileMenu implements 
 *            ScrollLayer the Layer returned is a (ScrollLayer*).
 *    @returns: Returns the (ScrollLayer*) Layer, NULL if uninitialised TileMenu.
 */
Layer *         tile_menu_get_layer(TileMenu * menu);

/**    Get Selected Tile 
 *    @brief: Gets the Layer of the currently visibly selected tile.
 *    @returns: Returns the Layer of the selected tile, NULL if uninitialised TileMenu.
 */
Layer *         tile_menu_get_selected(TileMenu * menu);
/**    Set Next Selected Tile
 *    @brief: Sets and moves the tile selector to the NEXT tile. This will loop
 *            back to the START tile if its currently selected on the END tile.
 *
 *    N.B. This will trigger a scroll event to occur if the NEXT tile is currently
 *         out of view and will shift the visible frame one tile row DOWN.
 */
void            tile_menu_set_selected_next(TileMenu * menu);
/**    Set Next Selected Tile
 *    @brief: Sets and moves the tile selector to the PREVIOUS tile. This will loop
 *            back to the END tile if its currently selected on the START tile.
 *
 *    N.B. This will trigger a scroll event to occur if the PREVIOUS tile is currently
 *         out of view and will shift the visible frame one tile row UP.
 */
void            tile_menu_set_selected_prev(TileMenu * menu);
