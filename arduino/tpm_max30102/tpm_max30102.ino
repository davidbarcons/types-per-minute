#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

/* 
To do: 
- Use a good algorithm for oxygen saturation calculation, e.g. https://www.analog.com/en/technical-articles/guidelines-for-spo2-measurement--maxim-integrated.html
- Implement a moving average filter for the heart rate.
*/

MAX30105 particleSensor;

const int BUFFER_SIZE = 4;  // Size of the buffer to store IR and red LED values. This averages the data.

uint16_t irBuffer[BUFFER_SIZE];    // Buffer to store IR values
uint16_t redBuffer[BUFFER_SIZE];   // Buffer to store red LED values
uint16_t beatsBuffer[BUFFER_SIZE];

long first_beat;
long second_beat;

float bpm;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY();
}

void loop()
{
  // Get the IR (infrared) and red LED values
  double irValue = particleSensor.getIR();
  double redValue = particleSensor.getRed();

  while (checkForBeat(irValue) == false)
  {
    //We sense the first beat and store the time
    irValue = particleSensor.getIR();
    first_beat = millis();
  }
  // We sense two more beats in between
  for (int i = 0; i<2; i++){
    while (checkForBeat(irValue) == false)
    {
      irValue = particleSensor.getIR();
    }
  }
  while (checkForBeat(irValue) == false)
  {
    //We sense the fourth beat and store the final time
    irValue = particleSensor.getIR();
    second_beat = millis();
  }
  // Calculate the BPM with 4 beats
  bpm = 60 / ((second_beat - first_beat) / 3000.0);
  
  // Store the IR and red LED values in the buffers
  for (int i = 1; i < BUFFER_SIZE; i++) {
    irBuffer[i-1] = irBuffer[i];
    redBuffer[i-1] = redBuffer[i];
    beatsBuffer[i-1] = beatsBuffer[i];
  }
  irBuffer[BUFFER_SIZE-1] = irValue;
  redBuffer[BUFFER_SIZE-1] = redValue;
  beatsBuffer[BUFFER_SIZE-1] = bpm;

  // Read the temperature
  float bodyTemperature = particleSensor.readTemperature();

  // Calculate oxygen saturation
  float oxygenSaturation = calculateOxygenSaturation();
  
  float beatsPerMinute = avgBuffer(beatsBuffer);
  // Print the measurements in csv format to be handled by our server
  Serial.print(beatsPerMinute);
  Serial.print("; ");
  Serial.print(oxygenSaturation);
  Serial.print("; ");
  Serial.println(bodyTemperature);
}

float calculateOxygenSaturation()
{
  // Calculate the ratio of red LED to IR values in the buffer
  long sumRed = 0;
  long sumIR = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    sumRed += redBuffer[i];
    sumIR += irBuffer[i];
  }
  float ratio = float(sumRed) / float(sumIR);

  // Perform calculations to estimate oxygen saturation
  // We should use calibration data and more sophisticated algorithms for better accuracy
  // e.g. https://www.analog.com/en/technical-articles/guidelines-for-spo2-measurement--maxim-integrated.html
  // This is a rude implementation
  float oxygenSaturation = 114 - 25 * ratio;

  return oxygenSaturation;
}

float avgBuffer(uint16_t buffer[BUFFER_SIZE])
{
  // Calculate the ratio of red LED to IR values in the buffer
  long avg = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    avg += buffer[i] / BUFFER_SIZE;
  }
  return avg;
}
