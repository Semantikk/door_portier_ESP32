//stand 30.09.2021, ohne Klingel
//https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
//https://randomnerdtutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/ <- Telegram Bot erstellen


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>


//-------------------------start settings-------------------------------------


const char* ssid = "FRITZ!Box 7530 MH";                         //Wlan Name
const char* password = "WLANKennwort";                          //Wlan passwort


#define BOTtoken "Telegram-Token"                               //Token
#define CHAT_ID "Telegram-Chat-ID"                              //chat ID

#define light 16                                                //linkes Relais, auf 230V Ausgelegt 
#define door 17                                                 //rechtes Relais

const int button = 14;                                          //Button-Pin Tür Alarm || zum deaktivieren Dip-Schalter 4 "ON"
int currentState;

int botRequestDelay = 1000;                                     //check SMS aller 1000ms
unsigned long lastTimeBotRan;

#define frequency_door_alarm 4                                  //alle 4 durchgänge wird eine SMS geschickt
int start = 0;


//-------------------------end settings-------------------------------------


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);


//////////////////////////////////////////////////////


void setup() {
  Serial.begin(115200);

  pinMode(door, OUTPUT);
  pinMode(light, OUTPUT);
  pinMode(button, INPUT);

  digitalWrite(door, HIGH);
  digitalWrite(light, HIGH);





  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
#endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}




//////////////////////////////////////////////////////


void loop() {
  delay(500);
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  handlebutton();

}


//////////////////////////////////////////////////////



void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;



    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your Home.\n\n";
      welcome += "/door  \n";
      welcome += "/light \n";
      welcome += "/options \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/options") {
      String keyboardJson = "[[\"/door\", \"/light\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Was kann ich tun?", "", keyboardJson, true);
    }

    if (text == "/door") {
      bot.sendMessage(chat_id, "Herzlich Wilkommen..", "");
      digitalWrite(light, LOW);
      delay(3000);
      digitalWrite(light, HIGH);
    }

    if (text == "/light") {
      bot.sendMessage(chat_id, "Es werde Licht..", "");
      digitalWrite(door, LOW);
      delay(500);
      digitalWrite(door, HIGH);
    }
  }

}

//////////////////////////////////////////////////////

void handlebutton() {
  currentState = digitalRead(button);

  if (currentState == LOW) {
    start++;
    Serial.println(start);

    if (start == frequency_door_alarm) {
      Serial.println("door is open");
      bot.sendMessage(CHAT_ID, "DOOR OPEN", "");
      start = 0;
    }
  }
}
