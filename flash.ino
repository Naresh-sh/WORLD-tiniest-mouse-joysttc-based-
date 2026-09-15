// Pico Joystick Mouse - LOW SENSITIVITY version
// Board: Raspberry Pi Pico
// Tools > USB Stack > Pico SDK (default, NOT Adafruit TinyUSB)

#include <Mouse.h>

const int JOY_X = 26;
const int JOY_Y = 27;
const int JOY_SW = 28;

int centerX = 2048;
int centerY = 2048;

// ---------- TUNE THESE VALUES TO ADJUST FEEL ----------
const int DEADZONE = 200;        // higher = less sensitive near center (try 100-250)
const float SENSITIVITY = 0.0035; // lower = slower cursor overall (try 0.002-0.006)
// -----------------------------------------------------------

const int MAX_SPEED = 5;         // lower top speed = less "fast flick" feel
const int SAMPLES = 6;           // how many readings to average, higher = smoother but slightly more lag

bool lastSwState = HIGH;

int readAveraged(int pin) {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(pin);
  }
  return sum / SAMPLES;
}

void setup() {
  pinMode(JOY_SW, INPUT_PULLUP);
  analogReadResolution(12);

  delay(200);
  centerX = readAveraged(JOY_X);
  centerY = readAveraged(JOY_Y);

  Mouse.begin();
}

void computeMove(int xRaw, int yRaw, int &moveX, int &moveY) {
  // work with the combined magnitude so diagonal and cardinal directions
  // get the same speed treatment, direction is preserved via ratio
  float magnitude = sqrt((float)xRaw * xRaw + (float)yRaw * yRaw);

  if (magnitude < DEADZONE) {
    moveX = 0;
    moveY = 0;
    return;
  }

  float normalized = magnitude / 2048.0;
  if (normalized > 1.0) normalized = 1.0;

  // cubed curve = gentler response at small tilt, still reaches
  // full speed at max tilt. Gives much finer control than squared.
  float curved = normalized * normalized * normalized;
  float curvedMagnitude = curved * 2048.0 * SENSITIVITY * 100.0;

  if (curvedMagnitude > MAX_SPEED) curvedMagnitude = MAX_SPEED;

  // scale x and y by the same factor so direction stays correct
  float scale = curvedMagnitude / magnitude;
  moveX = (int)(xRaw * scale);
  moveY = (int)(yRaw * scale);
}

void loop() {
  int xRaw = readAveraged(JOY_X) - centerX;
  int yRaw = readAveraged(JOY_Y) - centerY;

  int moveX = 0;
  int moveY = 0;

  computeMove(xRaw, yRaw, moveX, moveY);

  if (moveX != 0 || moveY != 0) {
    Mouse.move(moveX, moveY, 0);
  }

  bool swState = digitalRead(JOY_SW);
  if (swState == LOW && lastSwState == HIGH) {
    Mouse.press(MOUSE_LEFT);
  } else if (swState == HIGH && lastSwState == LOW) {
    Mouse.release(MOUSE_LEFT);
  }
  lastSwState = swState;

  delay(3); // slightly higher delay = calmer, less twitchy movement
}
