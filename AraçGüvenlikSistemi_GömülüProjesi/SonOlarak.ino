#include <TinyGPS++.h>//GPS modülü için kullanılan kütüphane
#include <SoftwareSerial.h>//Bu kütüphaneyle GPS modülüne seri port oluşturuldu
#include <WiFi.h>//WiFi modülü için kullanılan kütüphane
#include <WiFiClientSecure.h>//WiFi bağlantısı için kullanılan kütüphane
#include <UniversalTelegramBot.h> //Telegram bot API için kullanılan kütüphane   
#include <ArduinoJson.h>//JSON verileri işlemek için kullanılan kütüphane
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define BOTtoken "5554330972:AAFjTFcwHo_E6I8jbUpJnwWmHh9J7ItNaGA" 
#define CHAT_ID "5658959468"
#define WLAN_SSID "begummsdl"
#define WLAN_PASS "bmsdl2001"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883
#define IO_USERNAME  "begummsdl"
#define IO_KEY       "aio_TieT88k0FDYudG7u2BsGfBHBx4k6"

WiFiClientSecure client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);

const char* adafruitio_root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";

Adafruit_MQTT_Publish varSpeed = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/speed");
Adafruit_MQTT_Publish varLatitude = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/locationlatitude");
Adafruit_MQTT_Publish varLongitude = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/locationlongitude");

TinyGPSPlus gps;
SoftwareSerial SerialGPS(16, 17); 

UniversalTelegramBot bot(BOTtoken, client);

//Her 1 saniyede bir yeni mesaj olup olmadığını kontrol eder.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

float Latitude , Longitude;
int Speed, year, month, date, hour, minute, second;
String DateString, TimeString, LatitudeString, LongitudeString, SpeedString;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Yetkisiz kullanıcı", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Merhaba, " + from_name + ".\n";
      welcome += "Aracınızı kontrol etmek için aşağıdaki komutu kullanın.\n\n";
      welcome += "/location araç konumunu öğrenmek için \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/location") {
      String message;
      message = DateString + "   " + TimeString + "\n\n";
      message += "Araç hızınız: ";
      message += SpeedString + "\n\n";
      message += "Araç konumunuz: ";
      message += "http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=" + LatitudeString + "+" + LongitudeString;
      bot.sendMessage(chat_id, message, "");
    }
  }
}

WiFiServer server(80);
void setup()
{
  Serial.begin(115200);
  SerialGPS.begin(9600);

  Serial.println();
  Serial.print("Connecting");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); //Telegram API için gerekli sertifika ayarı
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
  
  client.setCACert(adafruitio_root_ca);
}
int lastSpeed=-1;
void loop()
{
  if (millis() > lastTimeBotRan + botRequestDelay)  { //Her 1 saniyede bir mesaj kontrolü
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read()))
    {
      if (gps.location.isValid())
      {
        Latitude = gps.location.lat();
        LatitudeString = String(Latitude , 6);
        Longitude = gps.location.lng();
        LongitudeString = String(Longitude , 6);
      }

      if (gps.speed.isValid()) {
        Speed = gps.speed.kmph();
        SpeedString = String(Speed);
        SpeedString += " km/h";
      }

      if (gps.date.isValid())
      {
        DateString = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();

        if (date < 10)
        DateString = '0';
        DateString += String(date);
        DateString += " / ";

        if (month < 10)
        DateString += '0';
        DateString += String(month);
        DateString += " / ";

        if (year < 10)
        DateString += '0';
        DateString += String(year);
      }

      if (gps.time.isValid())
      {
        TimeString = "";
        hour = gps.time.hour()+ 3; //UTC türkiye için 3
        minute = gps.time.minute();
        second = gps.time.second();
    
        if (hour < 10)
        TimeString = '0';
        TimeString += String(hour);
        TimeString += " : ";

        if (minute < 10)
        TimeString += '0';
        TimeString += String(minute);
        TimeString += " : ";

        if (second < 10)
        TimeString += '0';
        TimeString += String(second);
      }

    }
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html> <html> <head> <title>NEO-6M GPS Verileri</title><style>";
  s += "table, th, td {border: 1px solid blue;} </style> </head> <body> <h1  style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER>NEO-6M GPS Verileri</h1>";
  s += "<p ALIGN=CENTER style=""font-size:150%;""";
  s += "> <b>Konum Detaylari</b></p> <table ALIGN=CENTER style=";
  s += "width:50%";
  s += "> <tr> <th>Enlem</th>";
  s += "<td ALIGN=CENTER >";
  s += LatitudeString;
  s += "</td> </tr> <tr> <th>Boylam</th> <td ALIGN=CENTER >";
  s += LongitudeString;
  s += "</td> </tr> <tr>  <th>Tarih</th> <td ALIGN=CENTER >";
  s += DateString;
  s += "</td></tr> <tr> <th>Zaman</th> <td ALIGN=CENTER >";
  s += TimeString;
  s += "</td></tr> <tr> <th>Hiz(km/s)</th> <td ALIGN=CENTER >";
  s += SpeedString;
  s += "</td>  </tr> </table> ";
  
  if (gps.location.isValid())
  {
    s += "<p align=center>Konumun Google Haritalar'da acmak icin <a style=""color:RED;"" href=""http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
    s += LatitudeString;
    s += "+";
    s += LongitudeString;
    s += """ target=""_top"">tiklayiniz.</a></p>";
  }

  s += "</body> </html> \n";

  client.print(s);
  delay(100);
  
  MQTT_connect();
  if (lastSpeed!=Speed) {
    varSpeed.publish(Speed);
    varLatitude.publish(Latitude);
    varLongitude.publish(Longitude);
    Serial.print(Speed);
    Serial.println(" km/s");
    Serial.print("Enlem: ");
    Serial.println(Latitude);
    Serial.print("Boylam: ");
    Serial.println(Longitude);

    lastSpeed=Speed;
  }
  delay(2000);
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {return;}
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { 
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
}