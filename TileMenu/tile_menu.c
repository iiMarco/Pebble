/** TileMenu Layer
 *     Written By: Mark Zammit
 */
#include "tile_menu.h"
#include "xordll.h"

#define DIVIDE_UP(x,y)    (1 + ((x - 1) / y))
    
typedef struct _tile_menu_iterator_ {
    XORListIterator pointer;
    void * (*curr)(XORListIterator*);
    void * (*next)(XORListIterator*);
    void * (*prev)(XORListIterator*);
    bool   (*at_begin)(XORListIterator*);
    bool   (*at_end)(XORListIterator*);
} TileMenuIterator;
    
typedef struct _tile_menu_selector_ {
    InverterLayer * inverter;     // Inverted layer that acts as the visible selector
    TileMenuIterator iterator;    // Iterator to the selected Tile
    GPoint offset;                // Static offset, necessary to avoid animation interupts
} TileMenuSelector;
    
struct _tile_menu_ {
    ScrollLayer * layer;
    XORList * tiles;
    TileMenuIterator iterator;
    TileMenuSelector * selector;
    TileMenuCallback content_changed_handler;
    void * context;
    GPoint ulhs, lrhs;
};

void tile_menu_iterator_init(TileMenu * menu, TileMenuIterator * itr, bool forward);

void tile_menu_selector_create(TileMenu * menu);
void tile_menu_selector_destroy(TileMenuSelector * selector);
void tile_menu_selector_set(TileMenu * menu, TileMenuSelector * selector, Layer * parent, GRect from, GRect to);

static void tile_menu_content_offset_changed_handler(ScrollLayer * layer, void * context);

static void tile_menu_click_config_provider(void *context);
static void tile_menu_up_click_handler(ClickRecognizerRef recognizer, void *context);
static void tile_menu_down_click_handler(ClickRecognizerRef recognizer, void *context);
static void tile_menu_select_click_handler(ClickRecognizerRef recognizer, void *context);

static void tile_menu_content_offset_changed_handler(ScrollLayer * layer, void * context) {
    //...
}

static void tile_menu_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, tile_menu_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, tile_menu_down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, tile_menu_select_click_handler);
}

static void tile_menu_up_click_handler(ClickRecognizerRef recognizer, void *context) {
    tile_menu_set_selected_prev((TileMenu*)context);
}
 
static void tile_menu_down_click_handler(ClickRecognizerRef recognizer, void *context) {
    tile_menu_set_selected_next((TileMenu*)context);
}

static void tile_menu_select_click_handler(ClickRecognizerRef recognizer, void *context) {
    vibes_short_pulse();
}

void tile_menu_iterator_init(TileMenu * menu, TileMenuIterator * itr, bool forward) {
    if(!menu || !itr)
        return;
    
    itr->pointer = (forward ? xorlist_iterator_forward(menu->tiles) : xorlist_iterator_reverse(menu->tiles));
    itr->curr = &xorlist_iterator_curr;
    itr->next = (forward ? &xorlist_iterator_next : &xorlist_iterator_prev);
    itr->prev = (forward ? &xorlist_iterator_prev : &xorlist_iterator_next);
    itr->at_begin = (forward ? &xorlist_iterator_at_begin : &xorlist_iterator_at_end);
    itr->at_end = (forward ? &xorlist_iterator_at_end : &xorlist_iterator_at_begin);
}

void tile_menu_selector_create(TileMenu * menu) {
    if(!menu || menu->selector || xorlist_size(menu->tiles) <= 0)
        return;

    menu->selector = (TileMenuSelector*)malloc(sizeof(TileMenuSelector));
    menu->selector->offset = GPointZero;
    tile_menu_iterator_init(menu, &menu->selector->iterator, true);
    tile_menu_selector_set(menu,
                           menu->selector, 
                           scroll_layer_get_layer(menu->layer), 
                           layer_get_bounds((Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer))),
                           layer_get_bounds((Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer))));
}

void tile_menu_selector_destroy(TileMenuSelector * selector) {
    if(!selector)
        return;
    
    if(selector->inverter)
        inverter_layer_destroy(selector->inverter);
    
    free(selector);
}

void tile_menu_selector_set(TileMenu * menu, TileMenuSelector * selector, Layer * parent, GRect from, GRect to) {
    if(!selector || !parent)
        return;

    if(selector->inverter) {
        bool content_changed = false;
        GSize max_bounds = layer_get_bounds(parent).size;
        // Base offset
        GPoint offset = selector->offset;
        GRect start = from;
        GRect finish = to;
        GSize tile = to.size;
        
        start = GRect(start.origin.x,
                      start.origin.y + abs(offset.y),
                      start.size.w,
                      start.size.h);
        
        GPoint origin = start.origin;
        GPoint end = finish.origin;
        
        // Determines the relative left and right side boundaries
        //GRect bounds = layer_get_frame(parent);
        //int rhs = bounds.origin.x + bounds.size.w - tile.w;
        //int lhs = bounds.origin.x;

        // Relative on-screen start coordinates
        GPoint rel_start = from.origin;
        // Top and bottom True coordinates of the relative frame
        GPoint rframe_top = GPoint(0, origin.y - rel_start.y);
        GPoint rframe_bot = GPoint(0, origin.y + max_bounds.h - rel_start.y);
        // Relative on-screen end coordinates
        GPoint rel_end = GPoint(end.x, end.y);

        // Determined UP or DOWN shifting of the relative frame
        // Shift UP
        if(end.y < rframe_top.y) {
            offset = GPoint(0, rframe_top.y - end.y + offset.y);
            // Sets the new relative frame
            rframe_top.y -= rframe_top.y - abs(offset.y);
            rframe_bot.y = rframe_top.y + max_bounds.h;
            content_changed = true;
        }
        // Shift DOWN
        else if (end.y >= rframe_bot.y) {
            offset = GPoint(0, (-1 * (end.y - rframe_bot.y + tile.h)) + offset.y);
            // Sets the new relative frame
            rframe_top.y += abs(abs(offset.y) - rframe_top.y);
            rframe_bot.y = rframe_top.y + max_bounds.h;
            content_changed = true;
        }
        // Correct the relative end tile coordinates based on any movement
        rel_end = GPoint(rel_end.x, end.y - rframe_top.y);
        
        
        /* !! N.B. Animation currently DISABLED 
        // rhs = Right-hand-side last row tile
        // lhs = Left-hand-side first row tile
        // If moving from rhs --> lhs of any row then slide in from lhs
        // If moving from lhs --> rhs of any row then slide in from rhs
        // If moving from bot --> top then slide in from lhs
        // if moving from top --> bot then slide in from rhs
        if(finish.origin.x == lhs && start.origin.y != finish.origin.y) {
            start.origin.x = lhs - tile.w;
            start.origin.y = rel_end.y;
        } else if (finish.origin.x == rhs && start.origin.y != finish.origin.y) {
            start.origin.x = rhs + tile.w;
            start.origin.y = rel_end.y;
        } else if (gpoint_equal(&GPoint(finish.origin.x, finish.origin.y), &menu->ulhs) && 
                   gpoint_equal(&GPoint(start.origin.x, start.origin.y), &menu->lrhs)) {
            start.origin.x = lhs - tile.w;
            start.origin.y = rel_end.y;
        } else if (gpoint_equal(&GPoint(finish.origin.x, finish.origin.y), &menu->lrhs) && 
                   gpoint_equal(&GPoint(start.origin.x, start.origin.y), &menu->ulhs)) {
            start.origin.x = rhs + tile.w;
            start.origin.y = rel_end.y;
        }
        */
        GRect true_start = GRect(start.origin.x, rel_start.y, start.size.w, start.size.h);
        GRect true_end = GRect(finish.origin.x, rel_end.y, finish.size.w, finish.size.h);
        
        selector->offset = offset;
        scroll_layer_set_content_offset(menu->layer, offset, true);
        animate_layer(inverter_layer_get_layer(selector->inverter), &true_start, &true_end,0,0);
        
        if(content_changed && menu->content_changed_handler) {
            menu->content_changed_handler(menu, menu->context);
        }
    } else {
        selector->inverter = inverter_layer_create(to);
        layer_add_child(parent, inverter_layer_get_layer(selector->inverter));
    }
}



TileMenu * tile_menu_create(GRect frame, Window * window, unsigned tiles, unsigned tiles_per_view, unsigned tiles_per_row) {
    if(tiles_per_view == 0 || tiles_per_row == 0 || window == NULL)
        return NULL;
    
    int tile_height = frame.size.h / tiles_per_view;
    int tile_width = frame.size.w / tiles_per_row;
    GSize max_size = GSize (
        frame.size.w,
        tile_height * ((int)DIVIDE_UP(tiles,tiles_per_view)) < 
            frame.size.h ? frame.size.h : 
            tile_height * ((int)DIVIDE_UP(tiles,tiles_per_view))
        
    );

    TileMenu * menu = (TileMenu*)malloc(sizeof(struct _tile_menu_));
    
    menu->layer = scroll_layer_create(frame);
    menu->tiles = xorlist_create();
    menu->content_changed_handler = NULL;
    menu->context = menu;
    menu->ulhs = GPoint(frame.origin.x, frame.origin.y);
    menu->lrhs = menu->ulhs;
    
    for(unsigned i = 0; i < tiles; ++i) {
        unsigned col = i % tiles_per_row;
        unsigned row = DIVIDE_UP((i+1),tiles_per_view);
        
        GRect tile_bounds = GRect(
            frame.origin.x + (col * tile_width),
            frame.origin.y + ((row-1) * tile_height),
            tile_width,
            tile_height
        );

        menu->lrhs = GPoint(tile_bounds.origin.x, tile_bounds.origin.y);
        
        Layer * tile = layer_create(tile_bounds);
        xorlist_push_back(menu->tiles, (void*)tile);
    }
    
    scroll_layer_set_content_size(menu->layer, GSize(0, max_size.h));
    // Tile height offsets when srolling
    scroll_layer_set_content_offset(menu->layer, GPoint(0, tile_height), true);
    // Sets the default context to be TilemMenu*
    scroll_layer_set_context(menu->layer, (void*)menu);
    /*
    scroll_layer_set_callbacks(menu->layer, (ScrollLayerCallbacks) { 
        //.click_config_provider = &tile_menu_click_config_provider,
        .content_offset_changed_handler = tile_menu_content_offset_changed_handler
    });
    scroll_layer_set_click_config_onto_window(menu->layer, window);
    */
    window_set_click_config_provider_with_context(window, tile_menu_click_config_provider, (void*)menu);
    
    tile_menu_iterator_init(menu, &menu->iterator, true);
    tile_menu_selector_create(menu);
    
    return menu;
}

void tile_menu_destroy(TileMenu * menu) {
    if(menu) {
        tile_menu_selector_destroy(menu->selector);
        if(menu->tiles) {
            for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
                !xorlist_iterator_at_end(&itr);
                xorlist_iterator_next(&itr)) {
                layer_destroy((Layer*)xorlist_iterator_curr(&itr));
            }
        }
        xorlist_destroy(menu->tiles);
        scroll_layer_destroy(menu->layer);
    }
}

void tile_menu_draw(TileMenu * menu) {
    if(menu) {
        for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
            !xorlist_iterator_at_end(&itr);
            xorlist_iterator_next(&itr)) {
            Layer * layer = (Layer*)xorlist_iterator_curr(&itr);
            scroll_layer_add_child(menu->layer, layer);
        }
    }
}

void  tile_menu_set_context(TileMenu * menu, void * context) {
    if(!menu)
        return;
    menu->context = context;
}

void tile_menu_set_callbacks(TileMenu * menu, TileMenuCallbacks callbacks) {
    if(!menu)
        return;

    if(callbacks.click_config_provider && layer_get_window(scroll_layer_get_layer(menu->layer))) {
        window_set_click_config_provider_with_context(
            layer_get_window(scroll_layer_get_layer(menu->layer)), 
            callbacks.click_config_provider, 
            menu->context
        );
    }
    if(callbacks.content_changed_handler) {
        menu->content_changed_handler = callbacks.content_changed_handler;
    }
}

GRect tile_menu_get_bounds(TileMenu * menu) {
    return (menu ? layer_get_bounds(scroll_layer_get_layer(menu->layer)) : GRectZero);
}

Layer * tile_menu_get_next(TileMenu * menu) {
    if(!menu)
        return NULL; 
    if((*menu->iterator.at_end)(&menu->iterator.pointer)) {
        tile_menu_iterator_init(menu, &menu->iterator, true);
        return (Layer*)((*menu->iterator.curr)(&menu->iterator.pointer));
    }
    return (Layer*)((*menu->iterator.next)(&menu->iterator.pointer));
}

Layer * tile_menu_get_prev(TileMenu * menu) {
    if(!menu)
        return NULL;    
    if((*menu->iterator.at_begin)(&menu->iterator.pointer)) {
        tile_menu_iterator_init(menu, &menu->iterator, false);
        return (Layer*)((*menu->iterator.curr)(&menu->iterator.pointer));
    }
    return (Layer*)((*menu->iterator.prev)(&menu->iterator.pointer));
}

Layer * tile_menu_get_curr(TileMenu * menu) {
    return (menu ? (Layer*)((*menu->iterator.curr)(&menu->iterator.pointer)) : NULL);
}

int tile_menu_get_tile_count(TileMenu * menu) {
    return (menu ? xorlist_size(menu->tiles) : -1);
}

Window * tile_menu_get_window(TileMenu * menu) {
    return (menu ? layer_get_window(scroll_layer_get_layer(menu->layer)) : NULL);
}

Layer * tile_menu_get_layer(TileMenu * menu) {
    return (menu ? scroll_layer_get_layer(menu->layer) : NULL);
}

bool tile_menu_at_end(TileMenu * menu) {
    return (menu ? (*menu->iterator.at_end)(&menu->iterator.pointer) : true); 
}

bool tile_menu_at_begin(TileMenu * menu) {
    return (menu ? (*menu->iterator.at_begin)(&menu->iterator.pointer): true);
}

Layer * tile_menu_get_selected(TileMenu * menu) {
    return (menu && menu->selector ? (Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer)) : NULL);
}

void tile_menu_set_selected_next(TileMenu * menu) {
    if(!menu || !menu->selector)
        return;

    Layer * curr = (Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer));
    Layer * layer = (Layer*)((*menu->selector->iterator.next)(&menu->selector->iterator.pointer));
    if((*menu->selector->iterator.at_end)(&menu->selector->iterator.pointer)) {
        tile_menu_iterator_init(menu, &menu->selector->iterator, true);
        layer = (Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer));
    } 
    
    if(layer)
        tile_menu_selector_set(menu, menu->selector, scroll_layer_get_layer(menu->layer), layer_get_frame(curr), layer_get_frame(layer));
}

void tile_menu_set_selected_prev(TileMenu * menu) {
    if(!menu || !menu->selector)
        return;

    Layer * curr = (Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer));
    Layer * layer = (Layer*)((*menu->selector->iterator.prev)(&menu->selector->iterator.pointer));
    if((*menu->selector->iterator.at_begin)(&menu->selector->iterator.pointer)) {
        tile_menu_iterator_init(menu, &menu->selector->iterator, false);
        layer = (Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer));
    } 
    
    if(layer)
        tile_menu_selector_set(menu, menu->selector, scroll_layer_get_layer(menu->layer), layer_get_frame(curr), layer_get_frame(layer));
}
