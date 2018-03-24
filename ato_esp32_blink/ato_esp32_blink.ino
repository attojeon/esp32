/*
  ESP 32 Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
  The ESP32 has an internal blue LED at D2 (GPIO 02)
*/
int LED_BUILTIN = 2;

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() 
{

  for(int i=0; i<5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }

  delay(500);
  

}


