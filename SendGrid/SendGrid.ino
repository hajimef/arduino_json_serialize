#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "cer.h"

const char* ssid = "your_ssid";
const char* pass = "your_pass";
const char* api_key = "your_api_key";
const char* mail_to = "your_mail_to";
const char* mail_from = "your_mail_from";

#define SW_PIN 5

void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    yield();
    nowSecs = time(nullptr);
  }
}

void setup() {
  // シリアルポートの初期化 Initialize serial port
  Serial.begin(115200);
  // 5番ピンの初期化
  pinMode(SW_PIN, INPUT_PULLUP);
  // アクセスポイントに接続 Connect to access point
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  }
  // ESP32のIPアドレスを出力 Output IP address of ESP32
  Serial.println("WiFi Connected.");
  Serial.print("IP = ");
  Serial.println(WiFi.localIP());
  // 時刻を合わせる Set clock
  setClock();
}

void loop() {
  if (digitalRead(SW_PIN) == LOW) {
    Serial.println("Switch pushed");
    // WiFiClientSecureクラスのオブジェクトを生成する
    // Create WiFiClientSecure object
    WiFiClientSecure *client = new WiFiClientSecure;
    if(client) {
      // ルート証明書を設定する Set root CA
      client->setCACert(rootCA);
      {
        // メールのJSONを生成する Create JSON of mail
        StaticJsonDocument<500> doc;
        String output;
        
        JsonObject p = doc["personalizations"].createNestedObject();
        p["to"][0]["email"] = mail_to;
        p["subject"] = "Mail from SendGrid";
        doc["from"]["email"] = mail_from;
        JsonObject c = doc["content"].createNestedObject();
        c["type"] = "text/plain";
        c["value"] = "Hello, World!";
        serializeJson(doc, output);
        // SendGridでメールを送信する Send mail using SendGrid
        HTTPClient https;
        if (https.begin(*client, "https://api.sendgrid.com/v3/mail/send")) {
          // HTTPヘッダーを設定する set HTTP headers
          String auth = "Bearer ";
          auth += api_key;
          https.addHeader("Authorization", auth);
          https.addHeader("Content-Type", "application/json");
          // POSTでデータを送信 POST mail data
          Serial.println("Send mail start");
          int status = https.POST(output);
          Serial.println("Send mail end");
          if (status > 0) {
            if (status == HTTP_CODE_ACCEPTED) {
              // 通信OK send ok
              Serial.println("Send OK");
            }
            else {
              // HTTPエラー HTTP error
              Serial.print("HTTP Error ");
              Serial.println(status);
            }
          }
          else {
            // POSTエラー POST error
            Serial.println("POST Failed");
          }
          https.end();
        }
        else {
          // 接続エラー connect error
          Serial.print("Connect error");
        }
      }
      delete client;
    }
  }
}
