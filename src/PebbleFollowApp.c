/*
 * main.c
 * Creates a Window, Layer and animates it around the screen 
 * from corner to corner. It uses the `.stopped` callback to schedule the 
 * next stage of the animation.
 */

#include <pebble.h>
#include "PebbleFollowApp.h"
  
#define BOX_SIZE 20

#define ANIM_DURATION_STEP 50 //in ms
#define ANIM_DURATION_MAX_SCALE 40 //max delay 2 second
#define ANIM_DURATION_DELIMITER 1000 //delay 1 second between blobs
  
#define ANIM_DELAY 0
#define FONT_SIZE 35 //please match this with font name below
#define PEBBLE_FOLLOW_FONT_NAME FONT_KEY_BITHAM_30_BLACK
#define WINDOW_HEIGHT 168
#define WINDOW_WIDTH 144
  
#define APPROX_WORD_PER_LINE 8
  
static Window *s_main_window;
static TextLayer *s_text_flow_layer;
static PropertyAnimation *s_text_animation;
static GRect s_window_bounds = {{0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT}};
static int s_anim_duration_scale = 10;  //scale ANIM_DURATION_STEP

static TextBlob *s_text_blob;
static TextBlob** s_text_blobs;
static int s_text_blobs_size = 0;
static int s_text_blobs_pointer = 0;

typedef enum {
  TRANSITIONING,
  PAUSED,
  RUNNING,
  COMPLETED,
  STARTING,
} AppState;

static AppState s_app_state = STARTING;

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

int calculateAnimDuration(const char* word)
{
  int delay = s_anim_duration_scale * ANIM_DURATION_STEP;
  return delay;
}

static void next_animation() {
  // Schedule the next animation
  s_text_animation = property_animation_create_layer_frame( text_layer_get_layer(s_text_flow_layer), &s_window_bounds, &s_window_bounds);
  animation_set_curve((Animation*)s_text_animation, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)s_text_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  
  if(s_app_state == PAUSED)
  {
    animation_set_duration((Animation*)s_text_animation, ANIM_DURATION_DELIMITER);  
    animation_schedule((Animation*)s_text_animation);    
    return;
  }
  
  if(s_app_state == TRANSITIONING)
  {
    s_text_blobs_pointer++;
    s_app_state = RUNNING;
  }
  
  if (s_text_blobs_pointer < s_text_blobs_size)
  {
    const char* next_word = pebble_follow_text_blob_get_next_word(s_text_blobs[s_text_blobs_pointer]);
    if(next_word && (strcmp(next_word,"")))
    {
      animation_set_duration((Animation*)s_text_animation, calculateAnimDuration(next_word));
      text_layer_set_text(s_text_flow_layer, next_word);
    } else {
      animation_set_duration((Animation*)s_text_animation, ANIM_DURATION_DELIMITER);
      text_layer_set_text(s_text_flow_layer, "<=>");
      s_app_state = TRANSITIONING;
    }
    
    int approxTextHeight = FONT_SIZE * (s_text_blob->length % APPROX_WORD_PER_LINE);
      
    GRect new_bounds = GRect(0, (WINDOW_HEIGHT - approxTextHeight)/2, WINDOW_WIDTH, WINDOW_HEIGHT);
    layer_set_bounds(text_layer_get_layer(s_text_flow_layer), new_bounds);
    
  } else {
    s_app_state = COMPLETED;
    reset_blobs();
    s_app_state = RUNNING;
  }
  animation_schedule((Animation*)s_text_animation);
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

void destroy_text_blobs()
{
  if (s_text_blobs_size > 0)
  {
    for(int i=0; i<s_text_blobs_size; i++)
    {
      if(s_text_blobs[i])
        pebble_follow_text_blob_destroy(s_text_blobs[i]);
    }
    
    free(s_text_blobs);
  }
}

static void setup_sample_blobs()
{
  int size = 5;
  TextBlob **blobs;
  
  blobs = (TextBlob**) malloc(size * sizeof(TextBlob*));  
  
  pebble_follow_text_blob_create("Terrorist Attacks in France, Tunisia and Kuwait Kill Dozens", &blobs[0]);
  pebble_follow_text_blob_create("Protester Removes Confederate Flag at South Carolina Capitol", &blobs[1]);
  pebble_follow_text_blob_create("The Upshot: Where Same-Sex Couples Live", &blobs[2]);
  pebble_follow_text_blob_create("Jubilation, and Some Stalling, as Ruling Is Absorbed", &blobs[3]);
  pebble_follow_text_blob_create("Gunman Pursued Tourists in Slaughter at a Tunisian Hotel", &blobs[4]);
  
  
  pebble_follow_add_text_blobs(blobs, size);
}

static void reset_blobs()
{
  for (int i=0; i<s_text_blobs_size; i++)
    pebble_follow_text_blob_reset(s_text_blobs[i]);
  
  s_text_blobs_pointer = 0;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(s_anim_duration_scale > 1)
    s_anim_duration_scale--;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_app_state == PAUSED)
    s_app_state = RUNNING;
  else
    s_app_state = PAUSED;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(s_anim_duration_scale <= ANIM_DURATION_MAX_SCALE)
    s_anim_duration_scale++;
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
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
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_stack_push(s_main_window, true);

  setup_sample_blobs();
  pebble_follow_text_blob_create("Terrorist Attacks in France, Tunisia and Kuwait Kill Dozens", &s_text_blob);
  
  s_app_state = RUNNING;
  // Start animation loop
  next_animation();
}

static void deinit(void) {
  // Stop any animation in progress
  animation_unschedule_all();
  pebble_follow_text_blob_destroy(s_text_blob);
  destroy_text_blobs();
  
  // Destroy main Window
  window_destroy(s_main_window);
}


void pebble_follow_add_text_blobs(TextBlob** text_blobs, int size)
{
  //Destroy the already created blobs
  destroy_text_blobs();
  
  s_text_blobs = (TextBlob**) malloc(sizeof(TextBlob) * size);
  
  for (int i=0; i<size; i++)
    s_text_blobs[i] = text_blobs[i];
  
  s_text_blobs_size = size;
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

