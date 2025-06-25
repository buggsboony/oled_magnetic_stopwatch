//2025-06-24 23:46:36 - Power Management Headers
#include <avr/sleep.h> // Pour les fonctions de gestion de la veille
#include <avr/power.h> // Pour désactiver les périphériques inutiles (optimisation)

 
//2025-06-24 23:42:55 - OLED Screen header Stuff (Adafruit)
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h> //Adafruit License
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);




volatile bool magnetIsClose = true;
const byte reedSwitchPin = 3; // La pin D3 pour le capteur de porte

volatile unsigned long lastDebounceTime = 0; // Dernière fois que l'interrupteur a été déclenché
const unsigned long debounceDelay = 8; // Délai de débouncage en millisecondes (8ms est largement suffisant)

char sNumber[50];
short nCountDown;
short nMaxCountDown=60,zeroReached=0;
int delayTime=999;
bool first = true;

//2025-06-25 12:28:14 - Reset le countdown
void resetCountDown()
{
  nCountDown = nMaxCountDown;
  zeroReached = 0;
  delayTime = 999;
}//resetCountDown

// Fonction de service d'interruption (ISR)
void handleReedSwitch() {
  unsigned long currentTime = millis(); // Lit le temps actuel 
  // Lire l'état actuel de la pin
  int currentState = digitalRead(reedSwitchPin);

  if ((currentTime - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = currentTime; // Met à jour le dernier temps de déclenchement valide
      Serial.println("CHANGE !"); //Changement réel
  }else
  {
    Serial.print("!");
    return ;
  }//anti bounce
  if (currentState == HIGH) 
  { // La pin est passée à HIGH
    // Cela signifie que l'ILS est passé de fermé à ouvert.
    // D3 est tirée à HIGH par la pull-up, donc l'aimant est proche.
    magnetIsClose = true;
    Serial.println("INTERRUPTION: Aimant DETECTE (courant ne passe PLUS).");        
    resetCountDown();
  } else { // La pin est passée à LOW
    // Cela signifie que l'ILS est passé de ouvert à fermé.
    // D3 est tirée à LOW par le reed switch vers GND, donc l'aimant est loin.
    magnetIsClose = false;
    Serial.println("INTERRUPTION: Aimant ABSENT (courant PASSE).");
  }
}//handle reedswitch


// Function to put the microcontroller into deep sleep
void goToSleep() 
{
  Serial.println("Preparing to sleep..."); delay(5); //lui permet d'avoir le temps de finir d'écrire sur le serial.

  // Step 1: Disable all interrupts globally. This is CRITICAL.
  // Prevents any race conditions between enabling sleep and the actual sleep instruction.
  noInterrupts();

  // Step 2: Detach the current interrupt on D3. This clears any pending interrupt flags
  // and ensures no 'stale' interrupt mode is active.
  detachInterrupt(digitalPinToInterrupt(reedSwitchPin));

  // Step 3: Attach the interrupt again, but ONLY for the specific edge that will wake us up.
  // This prevents immediate re-wake-ups if the pin is already stable in one state.
  if (digitalRead(reedSwitchPin) == HIGH) { // Magnet is PROCHE -> pin is HIGH
    // We expect the magnet to move AWAY to wake up (pin goes HIGH to LOW)
    attachInterrupt(digitalPinToInterrupt(reedSwitchPin), handleReedSwitch, FALLING);
    Serial.println("Waiting for FALLING edge (magnet moving away) to wake up.");
  } else { // Magnet is LOIN -> pin is LOW
    // We expect the magnet to move CLOSER to wake up (pin goes LOW to HIGH)
    attachInterrupt(digitalPinToInterrupt(reedSwitchPin), handleReedSwitch, RISING);
    Serial.println("Waiting for RISING edge (magnet moving closer) to wake up.");
  }

  // Step 4: Set the sleep mode to POWER_DOWN (most power-efficient)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Step 5: Enable sleep mode
  sleep_enable();

  // Step 6: Disable unused peripherals to save power
  power_adc_disable();
  // Add other power_xxx_disable() calls here if you're not using SPI, TWI, etc.
  // power_spi_disable();
  // power_twi_disable();
  // If you disable power_timer0_disable(), millis() and delay() won't work while sleeping!
  // If you disable it, remember to re-enable it on wake-up.

  // Step 7: Re-enable global interrupts just before sleep_cpu()
  // This is necessary for the external interrupt on D3 to wake us up.
  interrupts();

  // Step 8: Enter sleep mode
  sleep_cpu();

  // --- Program execution resumes here after wake-up by interrupt ---

  // Step 9: Disable sleep mode IMMEDIATELY upon waking up
  sleep_disable();

  // Step 10: Disable global interrupts while we reset configurations
  noInterrupts();

  // Step 11: Re-enable peripherals if they were disabled
  power_adc_enable();
  // power_timer0_enable(); // Re-enable if disabled

  // Step 12: Re-attach the interrupt to CHANGE mode for general detection after wake-up
  detachInterrupt(digitalPinToInterrupt(reedSwitchPin)); // Detach the specific mode first
  attachInterrupt(digitalPinToInterrupt(reedSwitchPin), handleReedSwitch, CHANGE); // Re-attach to CHANGE

  // Step 13: Re-enable global interrupts for the rest of the program
  interrupts();
  Serial.println("Woke up!");
}//goToSleep_detach before



