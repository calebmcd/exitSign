#include <WiFi.h>
 
#include <Adafruit_MQTT.h>
 
#include <Adafruit_MQTT_Client.h>

#include <fastled.h>

#include <secrets.h>

#include <AsyncElegantOTA.h>

#include <elegantWebpage.h>

#include <hash.h>

// Fast LED Definitions
#define LED_PIN     2
#define NUM_LEDS    10
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

// AIO setup
WiFiClient client;     // Create an ESP8266 WiFiClient class to connect to the MQTT server.
 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);        // Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
 
Adafruit_MQTT_Subscribe LED_Control = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/exitSign_control");
 
void MQTT_connect();

bool greenFlag = false;

// setup OTA server
AsyncWebServer server(80);

// Flash a chosen color a certain number of times
void flashGreen(){
    
  for(int i=0;i<3;i++){

    fill_solid( leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(500);

    fill_solid( leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);
  }
}

void flashRed(){
    
  for(int i=0;i<3;i++){

    fill_solid( leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(500);

    fill_solid( leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(500);
  }
}

// ToDo: @calebmcd Figure out how to convert uint8_t* to CRGB member. Might not be possible.
struct CRGB toCRGB(uint8_t* inAIO){
  char* char_value = (char*)inAIO;

  //toupper(char_value[0]);

  //return CRGB::char_value;

}

void SetupBlackAndRedStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::Red;
    currentPalette[4] = CRGB::Red;
    currentPalette[8] = CRGB::Red;
    currentPalette[12] = CRGB::Red;
    
}

// Function to fill LED array
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
 
void setup() {

  delay(3000); // Power up delay

  // Setup LED Strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  
  // Set LED's to linear blend Rainbow
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
 
  Serial.begin(9600);
 
  // Connect to WiFi access point and set lights to Red
 
  Serial.println(); Serial.println();
 
  Serial.print("Connecting to ");
 
  Serial.println(WLAN_SSID);
  
  SetupBlackAndRedStripedPalette();
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
 
  while (WiFi.status() != WL_CONNECTED) {
 
    Serial.print(".");
    flashRed();
 
  }
 
  Serial.println();
 
 Serial.println("WiFi connected");
 
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
 
  mqtt.subscribe(&LED_Control);

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
 
}
 
uint32_t x=0;
 
void loop() {

  AsyncElegantOTA.loop();
 
  MQTT_connect();
 
Adafruit_MQTT_Subscribe *subscription;
 
  while ((subscription = mqtt.readSubscription(5000))) {
 
    if (subscription == &LED_Control) {
 
      Serial.print(F("Got: "));
 
      Serial.println((char *)LED_Control.lastread);
 
       if (!strcmp((char*) LED_Control.lastread, "ON"))
 
      {
 
        //Serial.println("Turned on");
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
        //fill_rainbow(leds, NUM_LEDS);
        FastLED.show();
 
      }
 
      else if (!strcmp((char*) LED_Control.lastread, "OFF"))
 
      {
 
        //Serial.println("Turned off");
        fill_solid( leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    
      }

      else {

        //const struct CRGB &color = toCRGB(LED_Control.lastread);

        toCRGB(LED_Control.lastread);

        //fill_solid( leds, NUM_LEDS, color);
        //FastLED.show();

      }
 
    }
 
  }
 
}
 
void MQTT_connect() {
 
  int8_t ret;
 
  // Stop if already connected.
 
  if (mqtt.connected()) {
 
    return;
 
  }
 
 Serial.print("Connecting to MQTT... ");
 
uint8_t retries = 3;
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
 
       Serial.println(mqtt.connectErrorString(ret));
 
       Serial.println("Retrying MQTT connection in 5 seconds...");
 
       mqtt.disconnect();
 
       delay(5000);  // wait 5 seconds
 
       retries--;
 
       if (retries == 0) {
 
         // basically die and wait for WDT to reset me
 
         while (1);
 
       }
 
  }
 
  Serial.println("MQTT Connected!");
  if(greenFlag==false) {
    flashGreen();
    greenFlag=true;
  }
 
}