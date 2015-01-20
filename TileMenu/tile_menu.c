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
    InverterLayer * inverter;
    TileMenuIterator iterator;
} TileMenuSelector;
    
struct _tile_menu_ {
    ScrollLayer * layer;
    XORList * tiles;
    TileMenuIterator iterator;
    TileMenuSelector * selector;
};

void tile_menu_iterator_init(TileMenu * menu, TileMenuIterator * itr, bool forward);

void tile_menu_selector_create(TileMenu * menu);
void tile_menu_selector_destroy(TileMenuSelector * selector);
void tile_menu_selector_set(TileMenuSelector * selector, Layer * parent, GRect frame);


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
    tile_menu_iterator_init(menu, &menu->selector->iterator, true);
    tile_menu_selector_set(menu->selector, 
                           scroll_layer_get_layer(menu->layer), 
                           layer_get_bounds((Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer))));
}

void tile_menu_selector_destroy(TileMenuSelector * selector) {
    if(!selector)
        return;
    
    if(selector->inverter)
        inverter_layer_destroy(selector->inverter);
    
    free(selector);
}

void tile_menu_selector_set(TileMenuSelector * selector, Layer * parent, GRect frame) {
    if(!selector || !parent)
        return;
    
    //if(start.origin.x == 0 && start.origin.y == 0 && 
    
    if(selector->inverter) {
        GRect start = layer_get_frame(inverter_layer_get_layer(selector->inverter));
        GRect finish = frame;
        animate_layer(inverter_layer_get_layer(selector->inverter), &start, &finish,200,0);
    } else {
        selector->inverter = inverter_layer_create(frame);
        layer_add_child(parent, inverter_layer_get_layer(selector->inverter));
    }
    
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting Selector: %p:%p @ {%d,%d,%d,%d}",
            parent,
            selector->inverter,
            frame.origin.x,
            frame.origin.y,
            frame.size.w,
            frame.size.h
    );
    
    
}



TileMenu * tile_menu_create(GRect frame, Window * window, unsigned tiles, unsigned tiles_per_view, unsigned tiles_per_row) {
    if(tiles_per_view == 0 || tiles_per_row == 0 || window == NULL)
        return NULL;
    
    TileMenu * menu = (TileMenu*)malloc(sizeof(struct _tile_menu_));
    
    menu->layer = scroll_layer_create(frame);
    menu->tiles = xorlist_create();
    
    int tile_height = frame.size.h / tiles_per_view;
    int tile_width = frame.size.w / tiles_per_row;
    
    layer_set_bounds(scroll_layer_get_layer(menu->layer), GRect(
        frame.origin.x,
        frame.origin.y,
        tile_height * ((int)DIVIDE_UP(tiles,tiles_per_row)) < 
            frame.size.h ? frame.size.h : 
            tile_height * ((int)DIVIDE_UP(tiles,tiles_per_row)),
        frame.size.w
    ));

    for(unsigned i = 0; i < tiles; ++i) {
        unsigned col = i % tiles_per_row;
        unsigned row = DIVIDE_UP((i+1),tiles_per_row);
        
        GRect tile_bounds = GRect(
            frame.origin.x + (col * tile_width),
            frame.origin.y + ((row-1) * tile_height),
            tile_width,
            tile_height
        );

        Layer * tile = layer_create(tile_bounds);
        /*
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating Tile: %p {%d,%d,%d,%d}", 
            text_layer,
            tile_bounds.origin.x,
            tile_bounds.origin.y,
            tile_bounds.size.h,
            tile_bounds.size.w
        ); 
        */
        xorlist_push_back(menu->tiles, (void*)tile);
    }
    
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

GRect tile_menu_get_bounds(TileMenu * menu) {
    return (menu ? layer_get_bounds(scroll_layer_get_layer(menu->layer)) : GRect(-1,-1,-1,-1));
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

    Layer * layer = (Layer*)((*menu->selector->iterator.next)(&menu->selector->iterator.pointer));
    
    if((*menu->selector->iterator.at_end)(&menu->selector->iterator.pointer)) {
        tile_menu_iterator_init(menu, &menu->selector->iterator, true);
        layer = (Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer));
    } 
    
    if(layer)
        tile_menu_selector_set(menu->selector, scroll_layer_get_layer(menu->layer), layer_get_frame(layer));
}

void tile_menu_set_selected_prev(TileMenu * menu) {
    if(!menu || !menu->selector)
        return;

    Layer * layer = (Layer*)((*menu->selector->iterator.prev)(&menu->selector->iterator.pointer));
    if((*menu->selector->iterator.at_begin)(&menu->selector->iterator.pointer)) {
        tile_menu_iterator_init(menu, &menu->selector->iterator, false);
        layer = (Layer*)((*menu->selector->iterator.curr)(&menu->selector->iterator.pointer));
    } 
    
    if(layer)
        tile_menu_selector_set(menu->selector, scroll_layer_get_layer(menu->layer), layer_get_frame(layer));
}
