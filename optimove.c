/*
 * main.c
 * Constructs a Window housing an output TextLayer to show data from 
 * either modes of operation of the accelerometer.
 */
#include <pebble.h>
#define MAXY 4000
#define MINY -4000
#define WINMAX 168
#define TAP_NOT_DATA false
static Window *s_main_window;
static TextLayer *s_output_layer;
static TextLayer *s_time_layer = NULL;
static int prev[3] = {0,0,0};
static int currPos[3] = {0,0,0};
static int numSteps = 0;

static void update_time() {
  // Get a tm structure
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  
}

static void data_handler(AccelData *data, uint32_t num_samples) {
  // Long lived buffer
  static char s_buffer[128];
  
  
  
  int avgX = (data[0].x + data[1].x + data[2].x)/3;
  int avgY = (data[0].y + data[1].y + data[2].y)/3;
  int avgZ = (data[0].z + data[1].z + data[2].z)/3;
  int XChang = abs(avgX - prev[0]);
  int YChang = abs(avgY - prev[1]);
  int ZChang = abs(avgZ - prev[2]);
  
  
  prev[0] = avgX;
  prev[1] = currPos[1] = avgY;
  prev[2] = avgZ;
  
  double val = (double)currPos[1]/(double)MAXY;
  
  int yPos = 60 + (WINMAX)*val;
  if (yPos > WINMAX - 32)
    yPos = WINMAX - 32;
  
  
  if (XChang > 200 && YChang > 200 && ZChang > 300) {
     numSteps++;
     snprintf(s_buffer, sizeof(s_buffer), "ActionPoints: %d", numSteps);
  }
  else {
     snprintf(s_buffer, sizeof(s_buffer), "ActionPoints: %d", numSteps);
  }
  //Show the data
  text_layer_set_text(s_output_layer, s_buffer);
  
  // Create text box
  
  
  if (s_time_layer != NULL)
    text_layer_destroy(s_time_layer);
  
  
  s_time_layer = text_layer_create(GRect(0, yPos, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  update_time();
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_time_layer));
  
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No data yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
  
  // Create time TextLayer for clock
  /*int yPos = window_bounds.size.h/2 + (window_bounds.size.h/2)*(currPos[1]/MAXY);
  if (yPos > window_bounds.size.h - 32)
    yPos = window_bounds.size.h - 32;
  if (s_time_layer != NULL)
    text_layer_destroy(s_time_layer);
  s_time_layer = text_layer_create(GRect(0, yPos, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  */
}
static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}
static void init() {
  // Create main Window
  s_main_window = window_create();
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
    
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Use tap service? If not, use data service
  // Subscribe to the accelerometer data service
  uint32_t num_samples = 3;
  accel_data_service_subscribe(num_samples, data_handler);
  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  
}
static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
  accel_data_service_unsubscribe();
}
int main(void) {
  init();
  app_event_loop();
  deinit();
}