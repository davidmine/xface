#include <pebble.h>

// State
static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_font36;
static GFont s_font18;
static AppTimer *s_print_timer;   // Used to schedule the print job
static char s_time[6];            // Used to hold the time
static char s_print_queue[11];    // Used to hold a portion of s_time for printing
static int s_buffer_count;        // Used to know which s_time character needs to be printed

// Internal prototypes
static void init();
static void deinit();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void update_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void print_time();
static void print_over_time(void *context);

/*********************************** Print ************************************/

static void print_over_time(void *context) {
  // I test if there's another character to print
  if(s_time[s_buffer_count] != '\0'){
    
    int random_delay = rand() % (600 + 1 - 200) + 200;
    
    // Grab characters from s_time and put them into the print_queue
    strncpy(s_print_queue, s_time, s_buffer_count + 1);
    
    // Terminate the print_queue
    s_print_queue[s_buffer_count + 1] = '\0';
    
    // Point at the next character in s_time
    s_buffer_count++;
    
    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_print_queue);
      
    // Print the next character after a delay
    s_print_timer = app_timer_register(random_delay, print_over_time, NULL);
    APP_LOG(APP_LOG_LEVEL_DEBUG, s_print_queue);
  }
}

static void print_time(){
  // Clear the screen
  s_print_queue[0] = '\0';

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_print_queue);

  // Print the next character after a delay
  s_print_timer = app_timer_register(500, print_over_time, NULL);
  APP_LOG(APP_LOG_LEVEL_DEBUG, s_print_queue);
}

/************************************ UI **************************************/

static void main_window_load(Window *window) {
  // Set window layer bounds
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create sizing layer
  // The goal of this layer is to calculate the vertical center
  TextLayer *vertical_position_layer = text_layer_create(bounds);
  text_layer_set_font(vertical_position_layer, s_font36);
  text_layer_set_text_alignment(vertical_position_layer, GTextAlignmentRight);
  text_layer_set_text(vertical_position_layer, "14:00");
  GSize vertical_position_size = text_layer_get_content_size(vertical_position_layer);

  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, (bounds.size.h / 2) - (vertical_position_size.h / 2), bounds.size.w-5, vertical_position_size.h + 10));

  // Modify text layer
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_font36);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);

  // Destroy the temporary layer
  text_layer_destroy(vertical_position_layer);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}


/*********************************** App **************************************/

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  strftime(s_time, sizeof(s_time), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  s_buffer_count = 0;
  print_time();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  // Load the custom font
  s_font36 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_XFILES_TYPE_46));
  s_font18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_XFILES_TYPE_18));
 
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
