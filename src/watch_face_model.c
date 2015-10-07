#include "watch_face_model.h"

// Generate a default watchface based on the current time.
static WatchFaceModel prv_watch_face_model_default(tm *tick_time) {
  const int32_t angle_minutes = tick_time->tm_min * TRIG_MAX_ANGLE / 60;
  const int32_t angle_hours = ((tick_time->tm_hour % 12) * TRIG_MAX_ANGLE + angle_minutes) / 12;

  return (WatchFaceModel) {
    .hour_hand = {
      .angle = angle_hours,
      .color = COLOR_HAND_HOUR,
      .length = LENGTH_HAND_HOUR,
      .width = WIDTH_HAND_HOUR,
    },
    .minute_hand = {
      .angle = angle_minutes,
      .color = COLOR_HAND_MINUTE,
      .length = LENGTH_HAND_MINUTE,
      .width = WIDTH_HAND_MINUTE,
    },
  };
}

// Tick time handler.
static void prv_handle_time_update(struct tm *tick_time, TimeUnits units_changed) {
  WatchFaceModel model = prv_watch_face_model_default(tick_time);
  watch_face_model_handle_change(&model);
}

// Initialize the watchface.
void watch_face_model_init() {
  WatchFaceModel model = {
    .hour_hand.length = 0,
    .minute_hand.length = 0,
  };
  watch_face_model_handle_change(&model);
}

/*** Animations ***/

// Perform linear interpolation between two 64-bit integers based on the progress of the animation.
int64_t interpolate_int64_linear(int64_t from, int64_t to, const AnimationProgress progress) {
  return from + ((progress * (to - from)) / ANIMATION_NORMALIZED_MAX);
}

// Perform linear interpolation between two watch hands based on the progress of the animation.
static WatchHandModel prv_interpolate_watch_hand_models(const WatchHandModel *from,
                                                        const WatchHandModel *to,
                                                        const AnimationProgress progress) {
  return (WatchHandModel) {
    .angle = (int32_t)interpolate_int64_linear(from->angle, to->angle, progress),
    .color = from->color,
    .length = (uint16_t)interpolate_int64_linear(from->length, to->length, progress),
    .width = from->width,
  };
}

// Perform linear interpolation between two watch faces based on the progress of the animation.
static WatchFaceModel prv_interpolate_watch_face_models(const WatchFaceModel *from,
                                                        const WatchFaceModel *to,
                                                        const AnimationProgress progress) {
  return (WatchFaceModel) {
    .minute_hand = prv_interpolate_watch_hand_models(&from->minute_hand, &to->minute_hand,
                                                     progress),
    .hour_hand = prv_interpolate_watch_hand_models(&from->hour_hand, &to->hour_hand, progress),
  };
}

// Update handler for intro animation.
static void prv_intro_animation_update(Animation *animation, const AnimationProgress progress) {
  time_t t = time(NULL);
  WatchFaceModel target_model = prv_watch_face_model_default(localtime(&t));

  WatchFaceModel start_model = target_model;
  start_model.hour_hand.length = 0;
  start_model.hour_hand.angle = target_model.hour_hand.angle - TRIG_MAX_ANGLE / 2;
  start_model.minute_hand.length = 0;
  start_model.minute_hand.angle = target_model.minute_hand.angle + TRIG_MAX_ANGLE / 2;

  WatchFaceModel model = prv_interpolate_watch_face_models(&start_model, &target_model, progress);
  watch_face_model_handle_change(&model);
}

// Teardown handler for the intro animation.
static void prv_intro_animation_finished(Animation *animation) {
  const time_t t = time(NULL);
  prv_handle_time_update(localtime(&t), (TimeUnits)0xff);
  tick_timer_service_subscribe(SECOND_UNIT, prv_handle_time_update);
}

// Start the intro animation for the watchface.
void watch_face_model_start_intro() {
  Animation *intro_animation = animation_create();
  static const AnimationImplementation s_intro_animation_impl = {
    .update = prv_intro_animation_update,
    .teardown = prv_intro_animation_finished,
  };

  animation_set_implementation(intro_animation, &s_intro_animation_impl);
  animation_set_curve(intro_animation, AnimationCurveEaseOut);
  animation_set_duration(intro_animation, 1000);
  animation_schedule(intro_animation);
}
