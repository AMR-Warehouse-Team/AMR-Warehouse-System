// =============================================================================
// PROJECT:  AMR 3-Sensor PID Line Follower
// HARDWARE: Arduino Uno + 2x L298N Motor Driver Modules + 4x DC Motors
//           + 3x IR Line Sensors (floor-facing, digital output)
// STEERING: Tank Drive (Skid Steering) — Left/Right motor groups
// CONTROL:  PID closed-loop via 3-sensor error signal
//
// PIN MAP — MOTORS:
//   Driver 1 (Left Side)   │  Driver 2 (Right Side)
//   LEFT_EN    → Pin 9     │  RIGHT_EN    → Pin 5  (PWM enable, speed)
//   LEFT_IN_FWD  → Pin 8   │  RIGHT_IN_FWD  → Pin 4 (direction: forward)
//   LEFT_IN_REV  → Pin 7   │  RIGHT_IN_REV  → Pin 2 (direction: reverse)
//
// PIN MAP — SENSORS:
//   SENSOR_LEFT   → A0  (HIGH = line detected under left sensor)
//   SENSOR_CENTER → A1  (HIGH = line detected under center sensor)
//   SENSOR_RIGHT  → A2  (HIGH = line detected under right sensor)
//
// AUTHOR:   [Your Name / Team Name]
// DATE:     [YYYY-MM-DD]
// VERSION:  2.0
// =============================================================================


// =============================================================================
// SECTION 1: PIN DEFINITIONS
// =============================================================================

// --- Driver 1: Left-Side Motors ---
#define LEFT_EN      9    // PWM: controls left motor speed (0–255)
#define LEFT_IN_FWD  8    // HIGH → left motors spin forward
#define LEFT_IN_REV  7    // HIGH → left motors spin backward

// --- Driver 2: Right-Side Motors ---
#define RIGHT_EN     5    // PWM: controls right motor speed (0–255)
#define RIGHT_IN_FWD 4    // HIGH → right motors spin forward
#define RIGHT_IN_REV 2    // HIGH → right motors spin backward

// --- IR Line Sensors (analog pins used as digital inputs) ---
#define SENSOR_LEFT   A0  // Left sensor
#define SENSOR_CENTER A1  // Center sensor
#define SENSOR_RIGHT  A2  // Right sensor


// =============================================================================
// SECTION 2: PID TUNING & GLOBAL STATE
//
// Tuning guide (tune one gain at a time):
//   1. Set Ki and Kd to 0. Raise Kp until the robot tracks the line but oscillates.
//   2. Raise Kd to dampen the oscillation without slowing correction.
//   3. Introduce Ki only if a steady-state offset persists after Kp/Kd are set.
//      Note: Ki is disabled (0.0) by default to prevent integrator windup.
//
// baseSpeed is a raw PWM value (0–255), not a percentage.
// At 100/255 (~39% duty cycle), speed is moderate — safe for initial tuning.
// =============================================================================

float Kp = 15.0;  // Proportional gain — primary correction strength
float Ki = 0.0;   // Integral gain    — corrects persistent steady-state error
float Kd = 10.0;  // Derivative gain  — dampens oscillation / overshoot

int   error         = 0;    // current positional error derived from sensor truth table
int   previousError = 0;    // error from the last loop cycle (used for D-term)
int   PID_Value     = 0;    // final PID output added/subtracted from each motor
float integral      = 0.0;  // running sum of error over time (I-term accumulator)

int baseSpeed = 100;  // base PWM duty cycle applied symmetrically to both motors


// =============================================================================
// SECTION 3: FUNCTION PROTOTYPES
// =============================================================================

void calculateError();
void calculatePID();
void applyMotorSpeeds();


// =============================================================================
// SECTION 4: SETUP
// =============================================================================

void setup() {
  Serial.begin(9600);
  Serial.println("=== AMR PID Line Follower — Initialized ===");

  // Motor control pins — all outputs
  pinMode(LEFT_EN,      OUTPUT);
  pinMode(LEFT_IN_FWD,  OUTPUT);
  pinMode(LEFT_IN_REV,  OUTPUT);
  pinMode(RIGHT_EN,     OUTPUT);
  pinMode(RIGHT_IN_FWD, OUTPUT);
  pinMode(RIGHT_IN_REV, OUTPUT);

  // Sensor pins — analog pins configured as digital inputs
  pinMode(SENSOR_LEFT,   INPUT);
  pinMode(SENSOR_CENTER, INPUT);
  pinMode(SENSOR_RIGHT,  INPUT);

  // Safe initial state: motors off, all direction pins cleared
  analogWrite(LEFT_EN,  0);
  analogWrite(RIGHT_EN, 0);
  digitalWrite(LEFT_IN_FWD,  LOW);
  digitalWrite(LEFT_IN_REV,  LOW);
  digitalWrite(RIGHT_IN_FWD, LOW);
  digitalWrite(RIGHT_IN_REV, LOW);
}


// =============================================================================
// SECTION 5: MAIN LOOP
// Each iteration executes one full PID cycle: sense → compute → act.
// There are no delays here — loop() must run as fast as possible so the
// derivative term reflects real-time rate-of-change.
// =============================================================================

void loop() {
  calculateError();
  calculatePID();
  applyMotorSpeeds();
}


// =============================================================================
// SECTION 6: SENSOR READING & ERROR CALCULATION
//
// Reads the three IR sensors via digitalRead and maps the detected pattern to a
// signed integer error. The sign encodes direction: negative = line is to the
// left of the robot, positive = line is to the right.
//
// Truth Table (L = left, C = center, R = right; 1 = line detected):
//
//   L   C   R  │  Error  │  Meaning
//  ────────────┼─────────┼─────────────────────────────────────────
//   0   1   0  │    0    │  Centered — robot is on the line
//   0   1   1  │   +1    │  Slight left drift  — line right of center
//   0   0   1  │   +2    │  Strong left drift  — line far to the right
//   1   1   0  │   -1    │  Slight right drift — line left of center
//   1   0   0  │   -2    │  Strong right drift — line far to the left
//   0   0   0  │   ±3   │  Line lost — spin toward last known direction
// =============================================================================

void calculateError() {
  int L = digitalRead(SENSOR_LEFT);
  int C = digitalRead(SENSOR_CENTER);
  int R = digitalRead(SENSOR_RIGHT);

  if      (L == 0 && C == 1 && R == 0) { error =  0; }  // centered
  else if (L == 0 && C == 1 && R == 1) { error =  1; }  // slight left drift
  else if (L == 0 && C == 0 && R == 1) { error =  2; }  // strong left drift
  else if (L == 1 && C == 1 && R == 0) { error = -1; }  // slight right drift
  else if (L == 1 && C == 0 && R == 0) { error = -2; }  // strong right drift
  else {
    // All sensors dark: line lost. Spin aggressively toward the last known side
    // so the robot sweeps back and re-acquires the line.
    error = (previousError > 0) ? 3 : -3;
  }
}


// =============================================================================
// SECTION 7: PID CALCULATION
//
// Standard PID formula applied to the positional error signal:
//   P (Proportional): reacts to the magnitude of the current error
//   I (Integral):     reacts to error that accumulates over time
//   D (Derivative):   reacts to how fast the error is changing — dampens swings
//
// previousError is updated here so it is always one cycle behind the current
// error, giving the D-term an accurate measure of the rate of change.
// =============================================================================

void calculatePID() {
  float P = error;
  integral += error;           // accumulate for I-term (has no effect while Ki == 0.0)
  float I = integral;
  float D = error - previousError;

  PID_Value = (int)((Kp * P) + (Ki * I) + (Kd * D));

  previousError = error;       // save for next cycle's D-term
}


// =============================================================================
// SECTION 8: MOTOR SPEED APPLICATION
//
// The PID output is applied differentially:
//   left  motor = baseSpeed + PID_Value  (positive PID → left speeds up)
//   right motor = baseSpeed - PID_Value  (positive PID → right slows down)
//
// This steers the robot toward the line without changing overall drive speed.
// Both values are clamped to [0, 255] — the valid 8-bit PWM range for analogWrite.
// Direction pins are set to forward on every call, ensuring they can never drift
// to a stale state from a prior operation.
// =============================================================================

void applyMotorSpeeds() {
  int leftMotorSpeed  = baseSpeed + PID_Value;
  int rightMotorSpeed = baseSpeed - PID_Value;

  // Clamp to valid PWM range
  leftMotorSpeed  = constrain(leftMotorSpeed,  0, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);

  // Lock both sides to the forward direction on every cycle
  digitalWrite(LEFT_IN_FWD,  HIGH);
  digitalWrite(LEFT_IN_REV,  LOW);
  digitalWrite(RIGHT_IN_FWD, HIGH);
  digitalWrite(RIGHT_IN_REV, LOW);

  // Write the differential speeds to the L298N enable pins
  analogWrite(LEFT_EN,  leftMotorSpeed);
  analogWrite(RIGHT_EN, rightMotorSpeed);
}
