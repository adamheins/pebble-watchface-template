#pragma once

#include <pebble.h>

#define COLOR_HAND_MINUTE GColorWhite
#define COLOR_HAND_HOUR GColorRed
#define COLOR_BACKGROUND GColorBlack

#define LENGTH_HAND_MINUTE 60
#define LENGTH_HAND_HOUR 40

#define WIDTH_HAND_MINUTE 3
#define WIDTH_HAND_HOUR 5

// Model of the watch hand.
typedef struct {
  int32_t angle;
  GColor color;
  uint16_t length;
  uint16_t width;
} WatchHandModel;

// Model of the watch face.
typedef struct {
  WatchHandModel minute_hand;
  WatchHandModel hour_hand;
} WatchFaceModel;

// Initialize the model.
void watch_face_model_init();

// Start the intro animation.
void watch_face_model_start_intro();

// Implemented by the client.
void watch_face_model_handle_change(WatchFaceModel *model);
