#include "tile_menu.h"
#include "xordll.h"

#define DIVIDE_UP(x,y)    (1 + ((x - 1) / y))
    
struct _tile_menu_ {
    ScrollLayer * layer;
    XORList * tiles;
    XORListIterator iterator;
};

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
                     
    //layer_add_child(window_get_root_layer(window), scroll_layer_get_layer(menu->layer));
    
    for(unsigned i = 0; i < tiles; ++i) {
        unsigned col = i % tiles_per_row;
        unsigned row = DIVIDE_UP((i+1),tiles_per_row);
        
        GRect tile_bounds = GRect(
            frame.origin.x + (col * tile_width),
            frame.origin.y + ((row-1) * tile_height),
            tile_width,
            tile_height
        );
        
        //TextLayer * tile = text_layer_create(tile_bounds);
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
    
     menu->iterator = xorlist_iterator_forward(menu->tiles);
    
    return menu;
}

void tile_menu_destroy(TileMenu * menu) {
    if(menu) {
        if(menu->tiles) {
            for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
                xorlist_iterator_has_next(&itr);
                xorlist_iterator_next(&itr)) {
                if(xorlist_iterator_curr(&itr)) {
                    layer_destroy((Layer*)xorlist_iterator_curr(&itr));
                    //text_layer_destroy((TextLayer*)xorlist_iterator_curr(&itr));
                }
            }
        }
        
        xorlist_destroy(menu->tiles);
        scroll_layer_destroy(menu->layer);
    }
}

void tile_menu_draw(TileMenu * menu) {
    if(menu) {
        for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
                xorlist_iterator_has_next(&itr);
                xorlist_iterator_next(&itr)) {
            //Layer * layer = text_layer_get_layer((TextLayer*)xorlist_iterator_curr(&itr));
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "Adding Layer: {%p}", text_layer); 
            Layer * layer = (Layer*)xorlist_iterator_curr(&itr);
            scroll_layer_add_child(menu->layer, layer);
        }
    }
}

GRect tile_menu_get_bounds(TileMenu * menu) {
    return (menu ? layer_get_bounds(scroll_layer_get_layer(menu->layer)) : GRect(-1,-1,-1,-1));
}

Layer * tile_menu_get_next(TileMenu * menu) {
    if(!xorlist_iterator_has_next(&menu->iterator))
        menu->iterator = xorlist_iterator_forward(menu->tiles);
    return xorlist_iterator_next(&menu->iterator);
}

Layer * tile_menu_get_prev(TileMenu * menu) {
    if(!xorlist_iterator_has_prev(&menu->iterator))
        menu->iterator = xorlist_iterator_reverse(menu->tiles);
    return xorlist_iterator_next(&menu->iterator);
}

Layer * tile_menu_get_curr(TileMenu * menu) {
    return xorlist_iterator_curr(&menu->iterator);
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
    return (menu ? !xorlist_iterator_has_next(&menu->iterator) : true); 
}

bool tile_menu_at_begin(TileMenu * menu) {
    return (menu ? !xorlist_iterator_has_prev(&menu->iterator) : true);
}

/*
void tile_menu_set_background_color(TileMenu * menu, GColor color) {
    if(menu)
        for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
                xorlist_iterator_has_next(&itr);
                xorlist_iterator_next(&itr)) {
            text_layer_set_background_color((TextLayer*)xorlist_iterator_curr(&itr), color);
        }
}

void tile_menu_set_font(TileMenu * menu, GFont font) {
    if(menu)
        for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
            xorlist_iterator_has_next(&itr);
            xorlist_iterator_next(&itr)) {
            text_layer_set_font((TextLayer*)xorlist_iterator_curr(&itr), font);
        }
}

void tile_menu_set_text(TileMenu * menu, const char * str) {
    if(menu)
        for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
            xorlist_iterator_has_next(&itr);
            xorlist_iterator_next(&itr)) {
            text_layer_set_text((TextLayer*)xorlist_iterator_curr(&itr), str);
        }
}

void tile_menu_set_text_alignment(TileMenu * menu, GTextAlignment alignment) {
    if(menu)
        for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
            xorlist_iterator_has_next(&itr);
            xorlist_iterator_next(&itr)) {
            text_layer_set_text_alignment((TextLayer*)xorlist_iterator_curr(&itr), alignment);
        }
}
*/

void tile_menu_set_text_color(TileMenu * menu, GColor color) {
    if(menu)
        for(XORListIterator itr = xorlist_iterator_forward(menu->tiles);
            xorlist_iterator_has_next(&itr);
            xorlist_iterator_next(&itr)) {
            text_layer_set_text_color((TextLayer*)xorlist_iterator_curr(&itr), color);
        }
}
