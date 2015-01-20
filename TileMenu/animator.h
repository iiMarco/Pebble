#include <pebble.h>
    
void on_animation_stopped(Animation *anim, bool finished, void *context);
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay);
