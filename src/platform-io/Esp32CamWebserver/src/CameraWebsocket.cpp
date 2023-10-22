#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "ESPAsyncWebServer.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"          //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include "driver/gpio.h"
#include <Preferences.h>

// configuration for AI Thinker Camera board
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

IPAddress LocalIp(192, 168, 1, 200);
IPAddress Gateway(192, 168, 1, 1);
IPAddress NMask(255, 255, 255, 0);

String ssid;
String password;

const char *hotspot_ssid = "ESP32-CAM";
const char *hotspot_password = "12345678";

String websockets_server_host;                // CHANGE HERE
const uint16_t websockets_server_port = 8000; // OPTIONAL CHANGE

camera_fb_t *fb = NULL;
size_t _jpg_buf_len = 0;
uint8_t *_jpg_buf = NULL;
uint8_t state = 0;
bool serverStarted = false;
int i = 0;
int statusCode;
String st;
String content;

using namespace websockets;
WebsocketsClient client;
AsyncWebServer server(80);

Preferences preferences;

bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for WiFi to connect");
  while (c < 20)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connection timed out, opening AP or Hotspot");
  return false;
}

void createWebServer()
{
  server.on("/", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              IPAddress ip;
              if (WiFi.getMode() == WIFI_AP)
              {
                ip = WiFi.softAPIP();
              }
              else
              {
                ip = WiFi.localIP();
              }
              String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
              content = "<!DOCTYPE HTML>\r\n<html>ESP32-CAM WiFi Connectivity Setup ";
              content += ipStr;
              content += "<form method='post' action='restart'><input type='submit' value='Restart'></form><p>";
              content += st;
              content += "</p><form method='get' action='setting'><label>SSID: </label><input required name='ssid' value=\"" + ssid + "\" length=32><br><label>PASS: </label><input required type='password' name='pass' value=\"" + password + "\" length=64><br><label>Websocket Address: </label><input required name='ip' value=\"" + websockets_server_host + "\" length=15><input type='submit'></form>";
              content += "</html>";
              request->send(200, "text/html", content);
            });
  server.on("/restart", HTTP_POST,
            [](AsyncWebServerRequest *request)
            {
              content = "{\"Success\":\"restarting...\"}";
              statusCode = 200;
              AsyncWebServerResponse *response = request->beginResponse(statusCode, "application/json", content); // Sends 404 File Not Found
              response->addHeader("Access-Control-Allow-Origin", "*");
              request->send(response);
              ESP.restart();
            });
  server.on("/setting", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
              int paramsNr = request->params();
              String query_ssid;
              String query_pass;
              String query_ip;
              for (int i = 0; i < paramsNr; i++)
              {
                AsyncWebParameter *p = request->getParam(i);
                if (p->name() == "ssid")
                {
                  query_ssid = p->value();
                }
                if (p->name() == "pass")
                {
                  query_pass = p->value();
                }
                if (p->name() == "ip")
                {
                  query_ip = p->value();
                }
              }
              if (query_ssid.length() > 0 && query_pass.length() > 0 && query_ip.length() > 0)
              {
                Serial.println("saving ssid:");
                preferences.putString("ssid", query_ssid);

                Serial.println("saving pass:");
                preferences.putString("password", query_pass);

                Serial.println("saving ip:");
                preferences.putString("ip", query_ip);

                content = "{\"Success\":\"saved... reset to boot into new wifi\"}";
                statusCode = 200;
                ESP.restart();
              }
              else
              {
                content = "{\"Error\":\"404 not found\"}";
                statusCode = 404;
                Serial.println("Sending 404");
              }
              AsyncWebServerResponse *response = request->beginResponse(statusCode, "application/json", content); // Sends 404 File Not Found
              response->addHeader("Access-Control-Allow-Origin", "*");
              request->send(response);
            });
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  serverStarted = true;
  Serial.println("Server started");
}

void scanWiFi(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan completed");
  if (n == 0)
    Serial.println("No WiFi Networks found");
  else
  {
    Serial.print(n);
    Serial.println(" Networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
}

void setupAP(void)
{
  WiFi.softAPConfig(LocalIp, Gateway, NMask);
  WiFi.softAP(hotspot_ssid, hotspot_password);
  Serial.println("Initializing_Wifi_accesspoint");
  launchWeb();
  Serial.println("over");
}

void onMessageCallback(WebsocketsMessage message)
{
  Serial.print("Got Message: ");
  Serial.println(message.data());
}

esp_err_t init_camera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // parameters for image quality and size
  config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 15;          // 10-63 lower number means higher quality
  config.fb_count = 2;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("camera init FAIL: 0x%x", err);
    return err;
  }
  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);
  Serial.println("camera init OK");
  return ESP_OK;
};

esp_err_t init_ws()
{
  Serial.println("connecting to WS: ");
  client.onMessage(onMessageCallback);
  bool connected = client.connect(websockets_server_host.c_str(), websockets_server_port, "/");
  if (!connected)
  {
    Serial.println("WS connect failed!");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  if (state == 3)
  {
    return ESP_FAIL;
  }

  Serial.println("WS OK");
  client.send("{\"message\": \"hello from ESP32 camera stream!\"}");
  return ESP_OK;
}

esp_err_t init_wifi()
{
  scanWiFi();
  delay(100);
  WiFi.config(LocalIp, Gateway, NMask);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Wifi init ");
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    init_ws();
    return ESP_OK;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    setupAP(); // Setup accesspoint or HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
  }

  return ESP_OK;
};

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  preferences.begin("config", false);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  websockets_server_host = preferences.getString("ip", "");

  init_camera();
  init_wifi();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!serverStarted)
      launchWeb();
    if (client.available())
    {
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb)
      {
        Serial.println("img capture failed");
        esp_camera_fb_return(fb);
        ESP.restart();
      }
      client.sendBinary((const char *)fb->buf, fb->len);
      Serial.println("image sent");
      esp_camera_fb_return(fb);
      client.poll();
    }
    else
    {
      Serial.println("WS not available");
      delay(1000);
      ESP.restart();
    }
  }
  else
  {
    Serial.println("Wifi disconnected");
    ESP.restart();
  }
}