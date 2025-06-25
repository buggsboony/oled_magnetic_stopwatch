#include "stopwatch.h"

const short verboz = 1;

void setup() 
{  
  Serial.begin(115200); // Initialise la communication série pour le débogage
  pinMode(reedSwitchPin, INPUT_PULLUP); // Configure la pin D3 comme entrée avec pull-up interne                                   
  // Attache l'interruption à la pin D3 (interruption 1)
  // CHANGE: Déclenche l'interruption à chaque changement d'état (montant ou descendant)
  attachInterrupt(digitalPinToInterrupt(reedSwitchPin), handleReedSwitch, CHANGE);
  // Vérifier l'état initial du reed switch
  if (digitalRead(reedSwitchPin) == HIGH) 
  { // Si D3 est HIGH, l'aimant est déjà proche
    magnetIsClose = true;
    Serial.println("Initialisation: Aimant PROCHE (switch non passant).");
  } else { // Si D3 est LOW, l'aimant est loin
    magnetIsClose = false;
    Serial.println("Initialisation: Aimant LOIN (switch passant).");
  }
  resetCountDown(); //peut eêtre utile
  Serial.println("Programme pret. Attente de changement d'etat de l'aimant.");

  // Serial.println("END SETUP");
  //     delay(200);
  //    Serial.println("go to sleep");
  //      delay(550);
  //    //if(okToSleep) .goToSleep();
     
 //2025-06-25 00:19:26 - Prepare Screen
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  //Pre-init 
  screenIsOf = false;
  okToSleep = magnetIsClose; //Sleep when magnet is closed
   
}//setup



void loop() 
{
  if(verboz>1) Serial.println("Start loop");
 

    if(first)
    {
      display.setTextSize(4);             // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE,SSD1306_BLACK ); // Draw 'inverse' text
      first=false;
    }

    //2025-06-25 13:04:12 - Allowed to sleep
    if(okToSleep) 
    {
      Serial.println("it's OK to sleep!");    
      //éteindre l'écran
      Serial.println("Turn OFF Screen");
      if(!screenIsOf)
      {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        screenIsOf = true;
      }
      goToSleep("FROM LOOP");
    }else
    {
      if(screenIsOf)
      {
        display.ssd1306_command(SSD1306_DISPLAYON);    
        screenIsOf = false;
        display.display();
      }
      Serial.println(" Not OK to sleep :(  "); 
    }
   
    if(verboz>1) Serial.println("Reset Cursor position 0,0");
    display.setCursor(0, 0);     // Re-Start at top-left corner
    display.print("  ");
    if(verboz>1) Serial.print("Print string number ");
    //Convert to string
    nCountDown--; //Decrement

    //Limit
    if(nCountDown<11)
    {
      nCountDown=11;
      zeroReached++;
      delayTime=200; //Blink rapidlzy
    }
    sprintf(sNumber,"%02d",nCountDown,3);
    //Pair ou impair
    if( (zeroReached>0) && ((zeroReached % 2) ==0)  )
    {
      if(verboz>1) Serial.print("modulo =");      
      //sprintf(sNumber,"--",n,3);  
      sprintf(sNumber,"  ",nCountDown,3);  
      //Serial.println("Go to sleep in loop ! ") ; delay(1100);       goToSleep();
    }

    Serial.println(sNumber);
    //display.println(n); 
    display.println(sNumber); 
  
    if(verboz>1) Serial.println("Display!");
    display.display();//Display on screen
    delay(delayTime); 

}//loop

 
