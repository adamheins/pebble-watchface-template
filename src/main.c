#include <pebble.h>

#include "watch_face_model.h"

static Window *window;
static WatchFaceModel s_model;

void watch_face_model_handle_change(WatchFaceModel *model) {
  s_model = *model;
  layer_mark_dirty(window_get_root_layer(window));
}

static void prv_draw_watch_hand(GContext *ctx, WatchHandModel *hand, GPoint center) {
  graphics_context_set_stroke_width(ctx, hand->width);
  graphics_context_set_stroke_color(ctx, hand->color);
  GPoint end = gpoint_from_polar(GRect(center.x  - hand->length, center.y - hand->length,
                                        hand->length * 2, hand->length * 2),
                                GOvalScaleModeFitCircle, hand->angle);
  graphics_draw_line(ctx, center, end);
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(window_get_root_layer(window));
  const GPoint center = grect_center_point(&bounds);

  // Background.
  graphics_context_set_fill_color(ctx, COLOR_BACKGROUND);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Watch hands.
  prv_draw_watch_hand(ctx, &s_model.hour_hand, center);
  prv_draw_watch_hand(ctx, &s_model.minute_hand, center);
}

static void window_load(Window *window) {
  watch_face_model_init();
  layer_set_update_proc(window_get_root_layer(window), prv_update_proc);
}

static void prv_app_did_focus(bool did_focus) {
  if (!did_focus) {
    return;
  }
  app_focus_service_unsubscribe();
  watch_face_model_start_intro();
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
  });
  window_stack_push(window, false);

  app_focus_service_subscribe_handlers((AppFocusHandlers) {
    .did_focus = prv_app_did_focus,
  });
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
