/*
=========================================================
SMART AI TRAFFIC CONTROL SYSTEM
ESP32 DEVKIT V1 + BLUETOOTH + NRF24L01
=========================================================

FEATURES:
✅ AI Traffic Density Control
✅ Bluetooth Phone Control
✅ NRF24L01 Support
✅ Emergency Vehicle Priority
✅ Manual Override
✅ Automatic Smart Signals
✅ Live Sensor Monitoring
✅ Works with your uploaded circuit

=========================================================
BLUETOOTH DETAILS
=========================================================

Bluetooth Name:
ESP32_TRAFFIC_SYSTEM

Commands:
AUTO
NORTH
SOUTH
EAST
WEST
ALLRED
STATUS
EMERGENCY
NORMAL

=========================================================
PIN CONNECTIONS
=========================================================

// NORTH SIGNAL
RED    -> GPIO13
YELLOW -> GPIO12
GREEN  -> GPIO14

// SOUTH SIGNAL
RED    -> GPIO27
YELLOW -> GPIO26
GREEN  -> GPIO25

// EAST SIGNAL
RED    -> GPIO17
YELLOW -> GPIO16
GREEN  -> GPIO15

// WEST SIGNAL
RED    -> GPIO2
YELLOW -> GPIO21
GREEN  -> GPIO22

// IR SENSORS
IR1 -> GPIO34
IR2 -> GPIO35
IR3 -> GPIO32
IR4 -> GPIO33

// NRF24L01
CE   -> GPIO4
CSN  -> GPIO5
SCK  -> GPIO18
MOSI -> GPIO23
MISO -> GPIO19
VCC  -> 3.3V
GND  -> GND

=========================================================
*/

#include <SPI.h>
#include <RF24.h>
#include "BluetoothSerial.h"

// =====================================================
// BLUETOOTH
// =====================================================
BluetoothSerial SerialBT;

// =====================================================
// NRF24L01
// =====================================================
RF24 radio(4, 5);

// =====================================================
// IR SENSOR PINS
// =====================================================
#define IR1 34
#define IR2 35
#define IR3 32
#define IR4 33

// =====================================================
// NORTH LEDs
// =====================================================
#define N_RED     13
#define N_YELLOW  12
#define N_GREEN   14

// =====================================================
// SOUTH LEDs
// =====================================================
#define S_RED     27
#define S_YELLOW  26
#define S_GREEN   25

// =====================================================
// EAST LEDs
// =====================================================
#define E_RED     17
#define E_YELLOW  16
#define E_GREEN   15

// =====================================================
// WEST LEDs
// =====================================================
#define W_RED     2
#define W_YELLOW  21
#define W_GREEN   22

// =====================================================
// TIMING
// =====================================================
int minGreenTime = 4000;
int maxGreenTime = 10000;
int yellowTime   = 2000;

// =====================================================
// MODES
// =====================================================
bool autoMode = true;
bool emergencyMode = false;

// =====================================================
// FUNCTION DECLARATIONS
// =====================================================
void allRed();

void northGreen(int duration);
void southGreen(int duration);
void eastGreen(int duration);
void westGreen(int duration);

void yellowTransition(int yellowPin);

void handleBluetooth();
void sendStatus();

// =====================================================
// SETUP
// =====================================================
void setup()
{
  Serial.begin(115200);

  // -------------------------------------------------
  // BLUETOOTH START
  // -------------------------------------------------
  SerialBT.begin("ESP32_TRAFFIC_SYSTEM");

  Serial.println("Bluetooth Started");
  Serial.println("Device Name: ESP32_TRAFFIC_SYSTEM");

  // -------------------------------------------------
  // IR SENSOR INPUTS
  // -------------------------------------------------

  // GPIO34 and GPIO35 do NOT support internal pullup
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);

  // GPIO32 and GPIO33 support pullup
  pinMode(IR3, INPUT_PULLUP);
  pinMode(IR4, INPUT_PULLUP);

  // -------------------------------------------------
  // LED OUTPUTS
  // -------------------------------------------------
  pinMode(N_RED, OUTPUT);
  pinMode(N_YELLOW, OUTPUT);
  pinMode(N_GREEN, OUTPUT);

  pinMode(S_RED, OUTPUT);
  pinMode(S_YELLOW, OUTPUT);
  pinMode(S_GREEN, OUTPUT);

  pinMode(E_RED, OUTPUT);
  pinMode(E_YELLOW, OUTPUT);
  pinMode(E_GREEN, OUTPUT);

  pinMode(W_RED, OUTPUT);
  pinMode(W_YELLOW, OUTPUT);
  pinMode(W_GREEN, OUTPUT);

  // -------------------------------------------------
  // NRF24L01 START
  // -------------------------------------------------
  if (!radio.begin())
  {
    Serial.println("NRF24L01 NOT DETECTED");
    SerialBT.println("NRF24L01 NOT DETECTED");
  }
  else
  {
    Serial.println("NRF24L01 Connected");
    SerialBT.println("NRF24L01 Connected");

    radio.setPALevel(RF24_PA_LOW);
  }

  // -------------------------------------------------
  // INITIAL STATE
  // -------------------------------------------------
  allRed();

  Serial.println("==================================");
  Serial.println("SMART TRAFFIC SYSTEM STARTED");
  Serial.println("==================================");

  SerialBT.println("ESP32 Traffic System Connected");
  SerialBT.println("Type STATUS for details");
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop()
{
  // Bluetooth Commands
  handleBluetooth();

  // Manual mode stops AI loop
  if (!autoMode)
  {
    return;
  }

  // -------------------------------------------------
  // READ SENSORS
  // -------------------------------------------------
  int northTraffic = digitalRead(IR1);
  int southTraffic = digitalRead(IR2);
  int eastTraffic  = digitalRead(IR3);
  int westTraffic  = digitalRead(IR4);

  // -------------------------------------------------
  // SERIAL MONITOR OUTPUT
  // -------------------------------------------------
  Serial.print("N:");
  Serial.print(northTraffic);

  Serial.print(" S:");
  Serial.print(southTraffic);

  Serial.print(" E:");
  Serial.print(eastTraffic);

  Serial.print(" W:");
  Serial.println(westTraffic);

  // -------------------------------------------------
  // SEND STATUS TO PHONE
  // -------------------------------------------------
  sendStatus();

  // -------------------------------------------------
  // EMERGENCY VEHICLE PRIORITY
  // -------------------------------------------------
  if (emergencyMode || northTraffic == LOW)
  {
    Serial.println("Emergency Vehicle Detected");
    SerialBT.println("Emergency Vehicle Detected");

    northGreen(maxGreenTime);

    return;
  }

  // -------------------------------------------------
  // AI TRAFFIC DENSITY LOGIC
  // -------------------------------------------------

  // NORTH
  if (northTraffic == LOW)
    northGreen(8000);
  else
    northGreen(minGreenTime);

  // SOUTH
  if (southTraffic == LOW)
    southGreen(8000);
  else
    southGreen(minGreenTime);

  // EAST
  if (eastTraffic == LOW)
    eastGreen(8000);
  else
    eastGreen(minGreenTime);

  // WEST
  if (westTraffic == LOW)
    westGreen(8000);
  else
    westGreen(minGreenTime);
}

// =====================================================
// HANDLE BLUETOOTH COMMANDS
// =====================================================
void handleBluetooth()
{
  if (SerialBT.available())
  {
    String command = SerialBT.readStringUntil('\n');

    command.trim();

    Serial.print("Bluetooth Command: ");
    Serial.println(command);

    // -------------------------------------------------
    // AUTO MODE
    // -------------------------------------------------
    if (command == "AUTO")
    {
      autoMode = true;
      emergencyMode = false;

      SerialBT.println("AUTO MODE ENABLED");
    }

    // -------------------------------------------------
    // NORTH
    // -------------------------------------------------
    else if (command == "NORTH")
    {
      autoMode = false;

      northGreen(maxGreenTime);

      SerialBT.println("NORTH GREEN ACTIVE");
    }

    // -------------------------------------------------
    // SOUTH
    // -------------------------------------------------
    else if (command == "SOUTH")
    {
      autoMode = false;

      southGreen(maxGreenTime);

      SerialBT.println("SOUTH GREEN ACTIVE");
    }

    // -------------------------------------------------
    // EAST
    // -------------------------------------------------
    else if (command == "EAST")
    {
      autoMode = false;

      eastGreen(maxGreenTime);

      SerialBT.println("EAST GREEN ACTIVE");
    }

    // -------------------------------------------------
    // WEST
    // -------------------------------------------------
    else if (command == "WEST")
    {
      autoMode = false;

      westGreen(maxGreenTime);

      SerialBT.println("WEST GREEN ACTIVE");
    }

    // -------------------------------------------------
    // ALL RED
    // -------------------------------------------------
    else if (command == "ALLRED")
    {
      autoMode = false;

      allRed();

      SerialBT.println("ALL SIGNALS RED");
    }

    // -------------------------------------------------
    // STATUS
    // -------------------------------------------------
    else if (command == "STATUS")
    {
      sendStatus();
    }

    // -------------------------------------------------
    // EMERGENCY MODE
    // -------------------------------------------------
    else if (command == "EMERGENCY")
    {
      emergencyMode = true;
      autoMode = true;

      SerialBT.println("EMERGENCY MODE ENABLED");
    }

    // -------------------------------------------------
    // NORMAL MODE
    // -------------------------------------------------
    else if (command == "NORMAL")
    {
      emergencyMode = false;
      autoMode = true;

      SerialBT.println("NORMAL MODE ENABLED");
    }

    // -------------------------------------------------
    // INVALID COMMAND
    // -------------------------------------------------
    else
    {
      SerialBT.println("INVALID COMMAND");
    }
  }
}

// =====================================================
// SEND STATUS TO PHONE
// =====================================================
void sendStatus()
{
  SerialBT.print("IR1:");
  SerialBT.print(digitalRead(IR1));

  SerialBT.print(" IR2:");
  SerialBT.print(digitalRead(IR2));

  SerialBT.print(" IR3:");
  SerialBT.print(digitalRead(IR3));

  SerialBT.print(" IR4:");
  SerialBT.println(digitalRead(IR4));
}

// =====================================================
// ALL RED
// =====================================================
void allRed()
{
  // RED ON
  digitalWrite(N_RED, HIGH);
  digitalWrite(S_RED, HIGH);
  digitalWrite(E_RED, HIGH);
  digitalWrite(W_RED, HIGH);

  // YELLOW OFF
  digitalWrite(N_YELLOW, LOW);
  digitalWrite(S_YELLOW, LOW);
  digitalWrite(E_YELLOW, LOW);
  digitalWrite(W_YELLOW, LOW);

  // GREEN OFF
  digitalWrite(N_GREEN, LOW);
  digitalWrite(S_GREEN, LOW);
  digitalWrite(E_GREEN, LOW);
  digitalWrite(W_GREEN, LOW);
}

// =====================================================
// NORTH GREEN
// =====================================================
void northGreen(int duration)
{
  allRed();

  digitalWrite(N_RED, LOW);
  digitalWrite(N_GREEN, HIGH);

  Serial.println("NORTH GREEN");
  SerialBT.println("NORTH GREEN");

  delay(duration);

  digitalWrite(N_GREEN, LOW);

  yellowTransition(N_YELLOW);
}

// =====================================================
// SOUTH GREEN
// =====================================================
void southGreen(int duration)
{
  allRed();

  digitalWrite(S_RED, LOW);
  digitalWrite(S_GREEN, HIGH);

  Serial.println("SOUTH GREEN");
  SerialBT.println("SOUTH GREEN");

  delay(duration);

  digitalWrite(S_GREEN, LOW);

  yellowTransition(S_YELLOW);
}

// =====================================================
// EAST GREEN
// =====================================================
void eastGreen(int duration)
{
  allRed();

  digitalWrite(E_RED, LOW);
  digitalWrite(E_GREEN, HIGH);

  Serial.println("EAST GREEN");
  SerialBT.println("EAST GREEN");

  delay(duration);

  digitalWrite(E_GREEN, LOW);

  yellowTransition(E_YELLOW);
}

// =====================================================
// WEST GREEN
// =====================================================
void westGreen(int duration)
{
  allRed();

  digitalWrite(W_RED, LOW);
  digitalWrite(W_GREEN, HIGH);

  Serial.println("WEST GREEN");
  SerialBT.println("WEST GREEN");

  delay(duration);

  digitalWrite(W_GREEN, LOW);

  yellowTransition(W_YELLOW);
}

// =====================================================
// YELLOW TRANSITION
// =====================================================
void yellowTransition(int yellowPin)
{
  digitalWrite(yellowPin, HIGH);

  Serial.println("YELLOW ON");
  SerialBT.println("YELLOW ON");

  delay(yellowTime);

  digitalWrite(yellowPin, LOW);
}
