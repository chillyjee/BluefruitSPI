/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/


#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"

#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true  // If set to 'true' enables debug output

#define BLUEFRUIT_SPI_CS               18
#define BLUEFRUIT_SPI_IRQ              11
#define BLUEFRUIT_SPI_RST              39    // Optional but recommended, set to -1 if unused

#define LED RED_LED

#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

/*=========================================================================
    APPLICATION SETTINGS

‚ÄÇ ‚ÄÇ FACTORYRESET_ENABLE‚ÄÇ ‚ÄÇ  Perform a factory reset when running this sketch
‚ÄÇ ‚ÄÇ
‚ÄÇ ‚ÄÇ                         Enabling this will put your Bluefruit LE module
                            in a 'known good' state and clear any config
                            data set in previous sketches or projects, so
‚ÄÇ ‚ÄÇ                         running this at least once is a good idea.
‚ÄÇ ‚ÄÇ
‚ÄÇ ‚ÄÇ                         When deploying your project, however, you will
                            want to disable factory reset by setting this
                            value to 0.‚ÄÇ If you are making changes to your
‚ÄÇ ‚ÄÇ                         Bluefruit LE device via AT commands, and those
                            changes aren't persisting across resets, this
                            is the reason why.‚ÄÇ Factory reset will erase
                            the non-volatile memory where config data is
                            stored, setting it back to factory default
                            values.
‚ÄÇ ‚ÄÇ ‚ÄÇ ‚ÄÇ
‚ÄÇ ‚ÄÇ                         Some sketches that require you to bond to a
                            central device (HID mouse, keyboard, etc.)
                            won't work at all with this feature enabled
                            since the factory reset will clear all of the
                            bonding data stored on the chip, meaning the
                            central device won't be able to reconnect.
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE      1
/*=========================================================================*/


// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
// NOTE: Reaplce &SPI with &SPI1 for the other hardware SPI port
Adafruit_BluefruitLE_SPI ble(&SPI, BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

bool showPrompt = true;

// A small helper
void error(const char *err) {
  Serial.println(err);
  while (1);
}

void error(const __FlashStringHelper *err) {
  Serial.println(err);
  while (1);
}
/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit AT Command Example"));
  Serial.println(F("-------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  
  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  // ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(1000);
  }
 
  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  ble.verbose(false);
  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  char message[BUFSIZE+1];
  ble.flush();
  Serial.flush();
  memset(message,0,sizeof(message));
  
  // Display command prompt
  Serial.print(F("\r\n> "));

  // Check for user input and echo it back if anything was found
  getUserInput(message, BUFSIZE);

  // Get the two first characters of the input to check if it is a command
  char twoFirstChars[2];
  memset(twoFirstChars,0,sizeof(twoFirstChars));
  strncat(twoFirstChars, message, 2);

  // Display reception flag
  Serial.print(F("* "));

  if (strcmp(twoFirstChars, "AT") ==  0) {
    // Print the command in the serial
    Serial.print("The command ");
    Serial.print(message);
    Serial.print(" has been sent!\r\n\r\n"); 
    // Send as a command
    ble.sendCommandCheckOK(message);    
  } else {
    // Transmit data message
    ble.println(message);   
    // Print the message in the serial
    Serial.print("The message \"");
    Serial.print(message);
    Serial.print("\" has been sent!\r\n");  
    // Check response status
    ble.waitForOK();
  }
}

/**************************************************************************/
/*!
    @brief  Checks for user input (via the Serial Monitor)
*/
/**************************************************************************/
void getUserInput(char inData[], uint8_t maxSize)
{
  memset(inData, 0, maxSize);
  while( Serial.available() == 0 ) {
    delay(1);
  }
 
 uint8_t inIndex = 0;
 while ( (inIndex < maxSize) && !(Serial.available() == 0) ) {
   if (Serial.available() > 0) {
     // read the incoming byte:
     inData[inIndex] = Serial.read();
     char c = inData[inIndex];
     if ((c == '\n') || (c == '\r')) {
       break;
     }
     Serial.print(inData[inIndex++]);
   }

 }
   Serial.print("\r\n"); 
}

