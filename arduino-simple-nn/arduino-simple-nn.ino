#include <TimerOne.h>
#include "neural_network.h"

#define DEBUG_PIN 9

/* Those weight were calculated by training the model */
double weights[] = {
  9.67299303,
  -0.2078435,
  -4.62963669};

/* Those are the inputs to test and benchmark */
double inputs[8][3] = {
    {0, 0, 0},
    {0, 0, 1},
    {0, 1, 0},
    {0, 1, 1},
    {1, 0, 0},
    {1, 0, 1},
    {1, 1, 0},
    {1, 1, 1},
};

String cmd = "";

void test_neural_network(void)
{
  for (int i=0; i<8; i++) {
    double result = nn_predict(inputs[i], weights, 3);
    Serial.print("result: ");
    Serial.println(result);
  }
  Serial.println();
}

void benchmark_neural_network(void)
{
  // Before run the NN prediction function, we're toggling
  // the pin twice, then by using the oscilloscope we measure
  // the time and then substract the T/2 from the time that
  // the prediction actually spend

  // First toggle
  digitalWrite(DEBUG_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(DEBUG_PIN, LOW);    // turn the LED off by making the voltage LOW

  // Second toggle
  digitalWrite(DEBUG_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(DEBUG_PIN, LOW);    // turn the LED off by making the voltage LOW

  // Benchmark the predict function
  digitalWrite(DEBUG_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  double result = nn_predict(inputs[0], weights, 3);
  digitalWrite(DEBUG_PIN, LOW);    // turn the LED off by making the voltage LOW

  Serial.print("DONE: ");
  Serial.println(result);
  delay(3000);                       // wait for a second
}

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("Started");
  
  // put your setup code here, to run once:
  pinMode(DEBUG_PIN, OUTPUT);

  // Timer initialize
  Timer1.initialize(3000000);
  Timer1.attachInterrupt(benchmark_neural_network);
  Timer1.stop();
//  Timer1.start();
}

void loop() {
//  benchmark_neural_network();

//  test_neural_network();
//  delay(3000);
  
  while(Serial.available()) {
    char tmp = Serial.read();
    cmd.concat(tmp);
  }

  if (cmd != "") {
    if(cmd.indexOf("TEST") >= 0) {
      cmd = "";
      test_neural_network();
    }
    else if(cmd.indexOf("START") >= 0) {
      cmd = "";
      Timer1.start();
    }
    else if(cmd.indexOf("STOP") >= 0) {
      Timer1.stop();
    }
  }
}
