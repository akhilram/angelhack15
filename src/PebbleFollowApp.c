/*
 * main.c
 * Creates a Window, Layer and animates it around the screen 
 * from corner to corner. It uses the `.stopped` callback to schedule the 
 * next stage of the animation.
 */

#include <pebble.h>

#define BOX_SIZE 20

#define ANIM_DURATION 500
#define ANIM_DELAY 300

static Window *s_main_window;
static Layer *s_box_layer;
static TextLayer *s_text_flow_layer;
static PropertyAnimation *s_box_animation;

static int s_current_stage = 0;

// Function prototype 
static void next_animation();

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
#ifdef PBL_PLATFORM_APLITE
  // Free the animation
  property_animation_destroy(s_box_animation);
#endif

  // Schedule the next one, unless the app is exiting
  if (finished) {
    next_animation();
  }
}

static void next_animation() {
  // Determine start and finish positions
  GRect start, finish;
  switch (s_current_stage) {
    case 0:
      start = GRect(0, 0, BOX_SIZE, BOX_SIZE);
      finish = GRect(144 - BOX_SIZE, 0, BOX_SIZE, BOX_SIZE);
      break;
    case 1:
      start = GRect(144 - BOX_SIZE, 0, BOX_SIZE, BOX_SIZE);
      finish = GRect(144 - BOX_SIZE, 168 - BOX_SIZE, BOX_SIZE, BOX_SIZE);
      break;
    case 2:
      start = GRect(144 - BOX_SIZE, 168 - BOX_SIZE, BOX_SIZE, BOX_SIZE);
      finish = GRect(0, 168 - BOX_SIZE, BOX_SIZE, BOX_SIZE);
      break;
    case 3:
      start = GRect(0, 168 - BOX_SIZE, BOX_SIZE, BOX_SIZE);
      finish = GRect(0, 0, BOX_SIZE, BOX_SIZE);
      break;
    default:
      start = GRect(0, 0, BOX_SIZE, BOX_SIZE);
      finish = GRect(0, 0, BOX_SIZE, BOX_SIZE);
      break;
  }

  // Schedule the next animation
  s_box_animation = property_animation_create_layer_frame(s_box_layer, &start, &finish);
  animation_set_duration((Animation*)s_box_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_box_animation, ANIM_DELAY);
  animation_set_curve((Animation*)s_box_animation, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)s_box_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  animation_schedule((Animation*)s_box_animation);

  if(s_current_stage==1)
    text_layer_set_text(s_text_flow_layer, "Test2");
  else if(s_current_stage==2)
    text_layer_set_text(s_text_flow_layer, "Test3");
  else
    text_layer_set_text(s_text_flow_layer, "Test4");
  
  // Increment stage and wrap
  s_current_stage = (s_current_stage + 1) % 4;
}

static void update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  s_text_flow_layer = text_layer_create(GRect(0, 0, 144, 154));
  text_layer_set_font(s_text_flow_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_text_flow_layer, GTextAlignmentCenter);
  text_layer_set_text(s_text_flow_layer, "Test!!");
  
  layer_add_child(window_layer, text_layer_get_layer(s_text_flow_layer));
  // Create Layer
  s_box_layer = layer_create(GRect(0, 0, BOX_SIZE, BOX_SIZE));
  layer_set_update_proc(s_box_layer, update_proc);
  layer_add_child(window_layer, s_box_layer);
}

static void main_window_unload(Window *window) {
  // Destroy Layer
  layer_destroy(s_box_layer);
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

  // Start animation loop
  next_animation();
}

static void deinit(void) {
  // Stop any animation in progress
  animation_unschedule_all();

  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
