/*
 * main.c
 * Creates a Window, Layer and animates it around the screen 
 * from corner to corner. It uses the `.stopped` callback to schedule the 
 * next stage of the animation.
 */

#include <pebble.h>
#include "TextBlob.h"
  
#define BOX_SIZE 20

#define ANIM_DURATION 500
#define ANIM_DELAY 0
#define FONT_SIZE 42 //please match this with font name below
#define PEBBLE_FOLLOW_FONT_NAME FONT_KEY_BITHAM_42_BOLD
#define WINDOW_HEIGHT 168
#define WINDOW_WIDTH 144
  
#define APPROX_WORD_PER_LINE 8
  
static Window *s_main_window;
static TextLayer *s_text_flow_layer;
static PropertyAnimation *s_text_animation;
static GRect s_window_bounds = {{0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT}};


static TextBlob *s_text_blob;

// Function prototype 
static void next_animation();

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
#ifdef PBL_PLATFORM_APLITE
  // Free the animation
  property_animation_destroy(s_text_animation);
#endif

  // Schedule the next one, unless the app is exiting
  if (finished) {
    next_animation();
  }
}

static void next_animation() {
  // Schedule the next animation
  s_text_animation = property_animation_create_layer_frame( text_layer_get_layer(s_text_flow_layer), &s_window_bounds, &s_window_bounds);
  animation_set_duration((Animation*)s_text_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_text_animation, ANIM_DELAY);
  animation_set_curve((Animation*)s_text_animation, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)s_text_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  
  const char* next_word = pebble_follow_text_blob_get_next_word(s_text_blob);
  
  if(next_word && (strcmp(next_word,"")))
  {
    text_layer_set_text(s_text_flow_layer, next_word);
    int approxTextHeight = FONT_SIZE * (s_text_blob->length % APPROX_WORD_PER_LINE);
    
    GRect new_bounds = GRect(0, (WINDOW_HEIGHT - approxTextHeight)/2, WINDOW_WIDTH, WINDOW_HEIGHT);
    layer_set_bounds(text_layer_get_layer(s_text_flow_layer), new_bounds);
    animation_schedule((Animation*)s_text_animation);
  }  
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  s_text_flow_layer = text_layer_create(s_window_bounds);
  text_layer_set_font(s_text_flow_layer, fonts_get_system_font(PEBBLE_FOLLOW_FONT_NAME));
	text_layer_set_text_alignment(s_text_flow_layer, GTextAlignmentCenter);
  text_layer_set_text(s_text_flow_layer, "");
  
  layer_add_child(window_layer, text_layer_get_layer(s_text_flow_layer));
}

static void main_window_unload(Window *window) {
  // Destroy Layer
  //layer_destroy(s_text_flow_layer); //TODO: Should I?
  text_layer_destroy(s_text_flow_layer);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
#ifdef PBL_SDK_2
  window_set_fullscreen(s_main_window, true);
#endif
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  pebble_follow_text_blob_create("Terrorist Attacks in France, Tunisia and Kuwait Kill Dozens", &s_text_blob);
  
  // Start animation loop
  next_animation();
}

static void deinit(void) {
  // Stop any animation in progress
  animation_unschedule_all();
  pebble_follow_text_blob_destroy(s_text_blob);
  
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

