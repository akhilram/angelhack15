/*
 * main.c
 * Creates a Window, Layer and animates it around the screen 
 * from corner to corner. It uses the `.stopped` callback to schedule the 
 * next stage of the animation.
 */

#include <pebble.h>
#include "PebbleFollowApp.h"
  
#define TEXT_BLOB_LIST_MAX_SIZE 20

#define ANIM_DURATION_STEP 40 //in ms
#define ANIM_DURATION_MAX_SCALE 40 //max delay 2 second
#define ANIM_DURATION_DELIMITER 1000 //delay 1 second between blobs
#define ANIM_WORD_DELAY 15  //10ms additional delay per letter
  
#define ANIM_DELAY 0
#define FONT_SIZE 35 //please match this with font name below
#define PEBBLE_FOLLOW_FONT_NAME FONT_KEY_BITHAM_30_BLACK
#define PEBBLE_HEADER_FONT FONT_KEY_GOTHIC_24_BOLD
#define WINDOW_HEIGHT 168
#define WINDOW_WIDTH 144
  
#define APPROX_WORD_PER_LINE 8
#define KEY_COUNT 0

static const char * const categories[] = { "Top Stories", "Most Popular", "Finance", "Twitter" };
static Window    *s_menu_window;
static MenuLayer *s_menu_layer;
   
static Window *s_main_window;
static TextLayer *s_text_flow_layer;
static TextLayer *s_text_header_layer;
static PropertyAnimation *s_text_animation;
static GRect s_header_bounds = {{0,0}, {WINDOW_WIDTH, 30}};
static GRect s_window_bounds = {{0,0}, {WINDOW_WIDTH, WINDOW_HEIGHT-30}};
static int s_anim_duration_scale = 4;  //scale ANIM_DURATION_STEP

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

//Menu Stuff
#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 4
  
static uint16_t pebble_follow_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t pebble_follow_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_MENU_ITEMS;
    default:
      return 0;
  }
}

static int16_t pebble_follow_menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void pebble_follow_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Categories");
      break;
  }
}

static void pebble_follow_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          // This is a basic menu item with a title and subtitle
          menu_cell_title_draw(ctx, cell_layer, "Top Stories");
          break;
        case 1:
          // This is a basic menu icon with a cycling icon
          menu_cell_title_draw(ctx, cell_layer, "Most Popular");
          break;
        case 2: 
          // This is a basic menu icon with a cycling icon
          menu_cell_title_draw(ctx, cell_layer, "Finance");
          break;
        case 3: 
          // This is a basic menu icon with a cycling icon
          menu_cell_title_draw(ctx, cell_layer, "Twitter");
          break;
      }
      break;
  }
}

enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1
};

// Write message to buffer & send
void send_message(int x){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, x);
	
	dict_write_end(iter);
  	app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Received Message: %s", tuple->value->cstring);
	}}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
  APP_LOG(APP_LOG_LEVEL_ERROR, "DROPPED");
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "ERROR");
}

static void pebble_follow_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  set_category(cell_index->row);
  reset_blobs();
  window_stack_pop(true);
//   APP_LOG(APP_LOG_LEVEL_INFO, category);
}
  
static void pebble_follow_menu_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  s_menu_layer = menu_layer_create(bounds);
  
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = pebble_follow_menu_get_num_sections_callback,
    .get_num_rows = pebble_follow_menu_get_num_rows_callback,
    .get_header_height = pebble_follow_menu_get_header_height_callback,
    .draw_header = pebble_follow_menu_draw_header_callback,
    .draw_row = pebble_follow_menu_draw_row_callback,
    .select_click = pebble_follow_menu_select_callback,
  });
  
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void pebble_follow_menu_window_unload(Window *window)
{
  menu_layer_destroy(s_menu_layer);
}

static void pebble_follow_init_menu()
{
  s_menu_window = window_create();
  window_set_window_handlers(s_menu_window, (WindowHandlers) {
    .load = pebble_follow_menu_window_load,
    .unload = pebble_follow_menu_window_unload,
  });

}


//Sync Stuff
static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
  // Update the TextLayer output
  APP_LOG(APP_LOG_LEVEL_INFO, "added string!");
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
  delay += strlen(word) * ANIM_WORD_DELAY;
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
    char* next_word = pebble_follow_text_blob_get_next_word(s_text_blob_list_pointer->blob);
    if(next_word && (strcmp(next_word,"")))
    {
      animation_set_duration((Animation*)s_text_animation, calculateAnimDuration(next_word));
      text_layer_set_text(s_text_flow_layer, next_word);
    } else {
      animation_set_duration((Animation*)s_text_animation, ANIM_DURATION_DELIMITER);
      //char *string_temp = "";
      //snprintf(string_temp,1000, "<=>:%d", s_list_size);
      text_layer_set_text(s_text_flow_layer, "<=>");
      s_app_state = TRANSITIONING;
    }

    int approxTextHeight = FONT_SIZE * (s_text_blob->length % APPROX_WORD_PER_LINE);
      
    GRect new_bounds = GRect(0, (WINDOW_HEIGHT - approxTextHeight)/2, WINDOW_WIDTH, WINDOW_HEIGHT);

//     int approxTextHeight = FONT_SIZE;//CHECK ME
      
//     GRect new_bounds = GRect(0, (WINDOW_HEIGHT - approxTextHeight)/2, WINDOW_WIDTH, WINDOW_HEIGHT);
    layer_set_bounds(text_layer_get_layer(s_text_flow_layer), new_bounds);
    
    free(next_word);
  } else {
    s_app_state = COMPLETED;
    reset_blobs();
    s_app_state = RUNNING;
  }
  animation_schedule((Animation*)s_text_animation);
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

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  s_text_header_layer = text_layer_create(s_header_bounds);
  text_layer_set_font(s_text_header_layer, fonts_get_system_font(PEBBLE_HEADER_FONT));
  text_layer_set_text_alignment(s_text_header_layer, GTextAlignmentCenter);
  text_layer_set_text(s_text_header_layer, categories[1]);
  
  s_text_flow_layer = text_layer_create(s_window_bounds);
  text_layer_set_font(s_text_flow_layer, fonts_get_system_font(PEBBLE_FOLLOW_FONT_NAME));
  text_layer_set_text_alignment(s_text_flow_layer, GTextAlignmentCenter);
  text_layer_set_text(s_text_flow_layer, "");
  
  layer_add_child(window_layer, text_layer_get_layer(s_text_header_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_text_flow_layer));
  
  setup_sample_blobs();
  s_app_state = RUNNING;
  // Start animation loop
  next_animation();
}

static void main_window_unload(Window *window) {
  // Destroy Layer
  //layer_destroy(s_text_flow_layer); //TODO: Should I?
  text_layer_destroy(s_text_flow_layer);
}

static void main_window_appear(Window *window) {
  s_app_state = RUNNING;
}

void destroy_text_blobs()
{
  pebble_follow_textbloblist_erase(s_text_blob_list_head);
  s_text_blob_list_head = NULL;
  s_text_blob_list_tail = NULL;
  s_text_blob_list_pointer = NULL;
  s_list_size = 0;
}

static void set_category(int category_index) {
  text_layer_set_text(s_text_header_layer, categories[category_index]);
  APP_LOG(APP_LOG_LEVEL_INFO, "Setting category");
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
  {
    s_app_state = PAUSED;
    window_stack_push(s_menu_window, true);  
  }
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
    .appear = main_window_appear
  });
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_stack_push(s_main_window, true);

  // Setup AppSync
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Setup initial values
  Tuplet initial_values[] = {
    TupletCString(KEY_COUNT, ""),
  };

  // Begin using AppSync
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_changed_handler, sync_error_handler, NULL);

  //Init Menu Window
  pebble_follow_init_menu();
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
  
  //pebble_follow_text_blob_list_purge();
  
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

