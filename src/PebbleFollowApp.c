/*
 * main.c
 * Creates a Window, Layer and animates it around the screen 
 * from corner to corner. It uses the `.stopped` callback to schedule the 
 * next stage of the animation.
 */

#include <pebble.h>
#include "PebbleFollowApp.h"

#define TEXT_BLOB_LIST_MAX_SIZE 20

#define ANIM_DURATION_STEP 50 //in ms
#define ANIM_DURATION_MAX_SCALE 40 //max delay 2 second
#define ANIM_DURATION_DELIMITER 1000 //delay 1 second between blobs
  
#define ANIM_DELAY 0
#define FONT_SIZE 35 //please match this with font name below
#define PEBBLE_FOLLOW_FONT_NAME FONT_KEY_BITHAM_30_BLACK
#define WINDOW_HEIGHT 168
#define WINDOW_WIDTH 144
  
#define APPROX_WORD_PER_LINE 8
#define KEY_COUNT 0

static Window *s_main_window;
static TextLayer *s_text_flow_layer;
static PropertyAnimation *s_text_animation;
static GRect s_window_bounds = {{0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT}};
static int s_anim_duration_scale = 10;  //scale ANIM_DURATION_STEP

static TextBlob *s_text_blob;
static TextBlobNode* s_text_blob_list_head = NULL;
static TextBlobNode* s_text_blob_list_tail = NULL;
static TextBlobNode* s_text_blob_list_pointer = NULL;
static int s_list_size = 0;

typedef enum {
  TRANSITIONING,
  PAUSED,
  RUNNING,
  COMPLETED,
  STARTING,
} AppState;

static AppState s_app_state = STARTING;

static AppSync s_sync;
static uint8_t s_sync_buffer[2048];

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
  // Update the TextLayer output
  pebble_follow_add_text_blob(new_tuple->value->cstring);
}

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  // An error occured!
  APP_LOG(APP_LOG_LEVEL_ERROR, "sync error! %d" , app_message_error);
}

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
    s_text_blob_list_pointer = s_text_blob_list_pointer->next;
    s_app_state = RUNNING;
  }
  
  if (s_text_blob_list_pointer != NULL)
  {
    const char* next_word = pebble_follow_text_blob_get_next_word(s_text_blob_list_pointer->blob);
    if(next_word && (strcmp(next_word,"")))
    {
      animation_set_duration((Animation*)s_text_animation, calculateAnimDuration(next_word));
      text_layer_set_text(s_text_flow_layer, next_word);
    } else {
      animation_set_duration((Animation*)s_text_animation, ANIM_DURATION_DELIMITER);
      char *string_temp = "";
      snprintf(string_temp,1000, "<=>:%d", s_list_size);
      text_layer_set_text(s_text_flow_layer, string_temp);
      s_app_state = TRANSITIONING;
    }

    int approxTextHeight = FONT_SIZE * (s_text_blob->length % APPROX_WORD_PER_LINE);
      
    GRect new_bounds = GRect(0, (WINDOW_HEIGHT - approxTextHeight)/2, WINDOW_WIDTH, WINDOW_HEIGHT);

//     int approxTextHeight = FONT_SIZE;//CHECK ME
      
//     GRect new_bounds = GRect(0, (WINDOW_HEIGHT - approxTextHeight)/2, WINDOW_WIDTH, WINDOW_HEIGHT);
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
  pebble_follow_textbloblist_erase(s_text_blob_list_head);
  s_text_blob_list_head = NULL;
  s_text_blob_list_tail = NULL;
  s_text_blob_list_pointer = NULL;
  s_list_size = 0;
}

static void setup_sample_blobs()
{
//  pebble_follow_add_text_blob("Terrorist Attacks in France, Tunisia and Kuwait Kill Dozens");  
//   pebble_follow_add_text_blob("Protester Removes Confederate Flag at South Carolina Capitol");
//   pebble_follow_add_text_blob("The Upshot: Where Same-Sex Couples Live");
//   pebble_follow_add_text_blob("Jubilation, and Some Stalling, as Ruling Is Absorbed");
//   pebble_follow_add_text_blob("Gunman Pursued Tourists in Slaughter at a Tunisian Hotel");  
  
  pebble_follow_text_blob_create("Terrorist Attacks in France, Tunisia and Kuwait Kill Dozens", &s_text_blob);
}

static void reset_blobs()
{
  TextBlobNode *curr = s_text_blob_list_head;
  while (curr!=NULL)
  {
    pebble_follow_text_blob_reset(curr->blob);
    curr = curr->next;
  }
  s_text_blob_list_pointer = s_text_blob_list_head;
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
  
  s_app_state = RUNNING;
  // Start animation loop
  next_animation();
  
    // Setup AppSync
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Setup initial values
  Tuplet initial_values[] = {
    TupletCString(KEY_COUNT, ""),
  };

  // Begin using AppSync
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_changed_handler, sync_error_handler, NULL);

}

static void deinit(void) {
  // Stop any animation in progress
  animation_unschedule_all();
  destroy_text_blobs();
  
  // Destroy main Window
  window_destroy(s_main_window);
  
  app_sync_deinit(&s_sync);
}

void pebble_follow_text_blob_list_purge()
{
  TextBlobNode *next = s_text_blob_list_head;
  while (s_list_size > TEXT_BLOB_LIST_MAX_SIZE && next != NULL)
  {
    next = s_text_blob_list_head->next;
    pebble_follow_text_blob_destroy(s_text_blob_list_head->blob);
    s_text_blob_list_head = next;
    s_list_size--;
  }
}

void pebble_follow_add_text_blob(const char* blobText)
{
  if(!blobText || !strcmp(blobText,""))
    return;
  
  pebble_follow_text_blob_list_purge();
  
  TextBlob *blob;    
  pebble_follow_text_blob_create(blobText, &blob);
  
  s_text_blob_list_tail = pebble_follow_textbloblist_push_back(s_text_blob_list_tail, blob);
  if(s_text_blob_list_head == NULL)
  {
    s_text_blob_list_head = s_text_blob_list_tail;
    s_text_blob_list_pointer = s_text_blob_list_head;
  }  
  
  s_list_size++;
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

