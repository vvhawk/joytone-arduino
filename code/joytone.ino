// Joystick RGB LED Distinct Quadrant Color Control

// Define Pins
#define BLUE 3
#define GREEN 5
#define RED 6
#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_BUTTON 2
#define BUZZER 8
#define POTENTIOMETER A2

// Color Rotation Variables
int redValue;
int greenValue;
int blueValue;
const int delayTime = 10; // fading time between colors
bool isRotating = false;

// Buzzer Variables
bool buzzerState = false; // Tracks whether the buzzer is on or off
bool lastButtonState = HIGH; // Tracks the last state of the joystick button
bool buttonState; // Current state of the joystick button

// Frequencies for "Happy Birthday"
int melody[] = {
  262, 262, 294, 262, 349, 330, // C, C, D, C, F, E
  262, 262, 294, 262, 392, 349, // C, C, D, C, G, F
  262, 262, 523, 440, 349, 330, 294, // C, C, C5, A, F, E, D
  466, 466, 440, 349, 392, 349 // Bb, Bb, A, F, G, F
};

int noteDurations[] = {
  500, 500, 1000, 1000, 1000, 2000, // Quarter notes, half notes
  500, 500, 1000, 1000, 1000, 2000, // Quarter notes, half notes
  500, 500, 1000, 1000, 1000, 1000, 2000, // Quarter notes, half notes
  500, 500, 1000, 1000, 1000, 2000 // Quarter notes, half notes
};

void setup() 
{
  // Initialize Serial for debugging
  Serial.begin(9600);
  
  // Set LED pins as outputs
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  
  // Set joystick pins
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);

  // Set buzzer pin as output
  pinMode(BUZZER, OUTPUT);

  // Set potentiometer pin as input
  pinMode(POTENTIOMETER, INPUT);
  
  // Initial color setup
  setColor(0, 0, 0);
}

void setColor(int red, int green, int blue) 
{
  // Directly write analog values
  analogWrite(RED, red);
  analogWrite(GREEN, green);
  analogWrite(BLUE, blue);
  
  // Update current color values
  redValue = red;
  greenValue = green;
  blueValue = blue;
}

void rotateColors() 
{
  // Only start rotation if not already rotating
  if (!isRotating) 
  {
    isRotating = true;
    
    // Red to Green
    for(int i = 0; i < 255; i += 1) 
    {
      redValue -= 1;
      greenValue += 1;
      analogWrite(RED, redValue);
      analogWrite(GREEN, greenValue);
      delay(delayTime);
    }

    // Green to Blue
    for(int i = 0; i < 255; i += 1) 
    {
      greenValue -= 1;
      blueValue += 1;
      analogWrite(GREEN, greenValue);
      analogWrite(BLUE, blueValue);
      delay(delayTime);
    }

    // Blue to Red
    for(int i = 0; i < 255; i += 1) 
    {
      blueValue -= 1;
      redValue += 1;
      analogWrite(BLUE, blueValue);
      analogWrite(RED, redValue);
      delay(delayTime);
    }
    
    // Reset flag and color
    isRotating = false;
    setColor(0, 0, 0);
  }
}

void determineQuadrantColor(int xValue, int yValue, int brightness) 
{
  // Quadrant thresholds
  int centerThreshold = 100;
  
  // Neutral (Center)
  if (abs(xValue - 512) < centerThreshold && abs(yValue - 512) < centerThreshold) 
  {
    setColor(0, 0, 0);
    //Serial.println("Neutral - Off");
    return;
  }
  
  // Determine primary quadrant
  if (abs(xValue - 512) > abs(yValue - 512)) 
  {
    // Horizontal dominance (Left/Right)
    if (xValue < 412) 
    {
      // Far Left - Pure Blue
      setColor(0, 0, brightness);
      //Serial.println("Left Quadrant - Pure Blue");
    } else if (xValue > 612) 
    {
      // Far Right - Pure Red
      setColor(brightness, 0, 0);
      //Serial.println("Right Quadrant - Pure Red");
    }
  } else 
  {
    // Vertical dominance (Top/Bottom)
    if (yValue > 612) 
    {
      // Top - Pure Green
      setColor(0, brightness, 0);
      //Serial.println("Top Quadrant - Pure Green");
    } else if (yValue < 412) 
    {
      // Bottom - Rotate Colors
      rotateColors();
      //Serial.println("Bottom Quadrant - Color Rotation");
    }
  }
}

void playTune() {
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    setColor(random(256), random(256), random(256));

    // Read potentiometer value for pitch control
    int potValue = analogRead(POTENTIOMETER);
    //int frequency = map(potValue, 0, 1023, 100, 2000); // Map to frequency range (100 Hz to 2000 Hz)
    float tempoScale = map(potValue, 0, 1023, 50, 200) / 100.0; // Map to tempo range (50% to 200%)

    // Adjust note duration based on frequency
    // int adjustedDuration = noteDurations[i] * frequency;

    // Adjust note duration based on tempo
    int adjustedDuration = noteDurations[i] * tempoScale;
    
    tone(BUZZER, melody[i]); // Play the accurate note
    // tone(BUZZER, frequency); // User picks note with POTENTIOMETER

    //delay(noteDurations[i]); // Hold the note for its accurate duration
    delay(adjustedDuration); // User picks duration with POTENTIOMETER

    noTone(BUZZER); // Stop the note
    delay(50); // Short pause between notes
  }
}

void loop() 
{

  // Read potentiometer value
  int potValue = analogRead(POTENTIOMETER);

  // Map potentiometer value to brightness (0–255)
  int brightness = map(potValue, 0, 1023, 0, 255);

  // Print potentiometer value to Serial Monitor
  // Serial.print("Potentiometer Value: ");
  // Serial.println(potValue);

  // Read joystick axes
  int xValue = analogRead(JOYSTICK_X);
  int yValue = analogRead(JOYSTICK_Y);
  
  // Determine and set color based on quadrant (with brightness scaling)
  determineQuadrantColor(xValue, yValue, brightness);

  if (digitalRead(JOYSTICK_BUTTON) == LOW) {
    playTune(); // Play the tune when the button is pressed
    delay(500); // Debounce delay
  }

  // Small delay to stabilize readings
  delay(50);
}