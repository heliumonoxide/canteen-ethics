#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include "ESP32_FTPClient.h"
#include <octocat.h>
#include "esp_camera.h"
#include <time.h>

// OV5640 camera module pins
//#define PWDN_GPIO_NUM    41
//#define RESET_GPIO_NUM   42
//#define XCLK_GPIO_NUM     15
//#define SIOD_GPIO_NUM     4
//#define SIOC_GPIO_NUM     5
//
//#define Y9_GPIO_NUM      16
//#define Y8_GPIO_NUM      17
//#define Y7_GPIO_NUM      18
//#define Y6_GPIO_NUM      12
//#define Y5_GPIO_NUM      10
//#define Y4_GPIO_NUM      8
//#define Y3_GPIO_NUM      9
//#define Y2_GPIO_NUM      11
//#define VSYNC_GPIO_NUM   6
//#define HREF_GPIO_NUM    7
//#define PCLK_GPIO_NUM    13

// OV2640 camera module pins
#define PWDN_GPIO_NUM    38
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y9_GPIO_NUM      16
#define Y8_GPIO_NUM      17
#define Y7_GPIO_NUM      18
#define Y6_GPIO_NUM      12
#define Y5_GPIO_NUM      10
#define Y4_GPIO_NUM      8
#define Y3_GPIO_NUM      9
#define Y2_GPIO_NUM      11
#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM    13

// Thanks to NORVI for supplying these configuration of ESP32 S3 with OV5640 https://www.esp32.com/viewtopic.php?f=17&t=40731 information: https://github.com/IndustrialArduino/NORVI-ESP32-CAMERA
// Also big thanks to Leonardo Bispo in https://github.com/ldab for developing ESP32_FTPClient
// Before upload to your esp32 cam s3. make sure you use this in tools:
// ESP32S3 Dev Module as board. Use OPI PSRAM (as PSRAM). the rest use default.

//#define WIFI_SSID "dapoersemar" // Your wifi name
//#define WIFI_PASS "warkopds" // Your wifi password

const char* ssid = "dapoersemar";
const char* password = "warkopds";

// Please make sure your wifi can access internet and have bandwidth since esp will be client of the wifi.

char ftp_server[] = "34.50.84.251";
uint16_t ftp_port = 21; // Usually used for FTP connection in port 21
char ftp_user[]   = "hazel";
char ftp_pass[]   = "Ngg@kt4uSaya";

ESP32_FTPClient ftp(ftp_server, ftp_port, ftp_user, ftp_pass, 5000, 2); // timeout 5 sec and verbose 2 (for debugging version)

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());

//  // Configure time and set timezone
//  configTime(25200, 0, "pool.ntp.org"); // Use NTP to sync time to jakarta time +7hours
//
//  // Ensure time is synced
//  struct tm timeinfo;
//  if (!getLocalTime(&timeinfo)) {
//    Serial.println("Failed to obtain time");
//    return;
//  }

  // Configure time and set timezone
  configTime(25200, 0, "pool.ntp.org");
  
  // Wait for time to sync
  Serial.println("Waiting for time sync...");
  struct tm timeinfo;
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 10) {
    delay(1000);  // Wait for 1 second
    retries++;
    Serial.println("Retrying to obtain time...");
  }
  
  if (retries == 10) {
    Serial.println("Failed to obtain time after 10 attempts");
    return;
  } else {
    Serial.println("Time synchronized successfully!");
  }

  // OV2640 camera module
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) {
    Serial.println("PSRAM Found.");
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    Serial.println("PSRAM Not Found.");
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // camera init
  Serial.printf("Free heap before camera init: %d\n", ESP.getFreeHeap());
  Serial.println("Starting camera initialization...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 2);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 1);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 2);       // -2 to 2
  s->set_aec_value(s, 800);    // 0 to 1200
  s->set_gain_ctrl(s, 0);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)6);  // 0 to 6
  s->set_bpc(s, 1);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 1);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

  Serial.println("Camera ready");
}

//void loop() {
//  // Capture the image
//  camera_fb_t *fb = esp_camera_fb_get();
////  if (!fb) {
////    Serial.println("Camera capture failed");
////    delay(3000);
////    return;
////  }
//  if (!fb) {
//        Serial.println("Camera capture failed!");
//        delay(3000);
//        return;
//    } else {
//        Serial.printf("Captured frame: %d bytes\n", fb->len);
//        esp_camera_fb_return(fb);
//    }
//
//  //  // Send the image size over Serial
//  //  Serial.write((uint8_t*)&fb->len, 4);
//  //
//  //  // Send the image data over Serial
//  //  Serial.write(fb->buf, fb->len);
//  //
//  //  // Return the frame buffer back to the driver for reuse
//  //  esp_camera_fb_return(fb);
//
//  struct tm timeinfo;
//  if (!getLocalTime(&timeinfo)) {
//    Serial.println("Failed to obtain time");
//  }
//
//  // Create a buffer to hold the formatted time
//  char filename[30];
//  strftime(filename, sizeof(filename), "image_%Y%m%d_%H%M%S.jpg", &timeinfo);
//
//  Serial.println("Generated file name: " + String(filename));
//
//  // Connect to FTP server
//  ftp.OpenConnection();
//  Serial.println("Connected to FTP server.");
//
//  // Create or navigate to the desired FTP directory
//  ftp.ChangeWorkDir("/esp_images");
//
//  // Send the image as a .jpg file
//  ftp.InitFile("Type I");  // Switch to binary mode
//  ftp.NewFile(filename);
//
//  ftp.WriteData((unsigned char*)fb->buf, fb->len);  // Send JPEG buffer
//  ftp.CloseFile();  // Finalize the file transfer
//  Serial.println("Done writing data to jpg image");
//  
//  ftp.CloseConnection();
//  esp_camera_fb_return(fb);  // Release frame buffer
//
//
//  delay(59000); // Capture every 60 seconds
//}

void loop() {
  // Capture the image
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    delay(3000);
    return;
  }

  // Send the image size and data to FTP
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    esp_camera_fb_return(fb);
    return;
  }

  // Create a filename using the current time
  char filename[30];
  strftime(filename, sizeof(filename), "image_%Y%m%d_%H%M%S.jpg", &timeinfo);
  Serial.println("Generated file name: " + String(filename));

  // FTP Upload Process
  ftp.OpenConnection();
  Serial.println("Connected to FTP server.");
  ftp.ChangeWorkDir("/esp_images");
  ftp.InitFile("Type I");  // Switch to binary mode
  ftp.NewFile(filename);
  ftp.WriteData((unsigned char*)fb->buf, fb->len);  // Send JPEG buffer
  ftp.CloseFile();
  Serial.println("Done writing data to jpg image");

  // Close FTP connection
  ftp.CloseConnection();
  
  // Release the frame buffer
  esp_camera_fb_return(fb);

  // Wait before next capture
  delay(59000); // Capture every 60 seconds
}
