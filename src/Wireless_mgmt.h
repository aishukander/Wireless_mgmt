#ifndef WIRELESS_MGMT
#define WIRELESS_MGMT

#include <Arduino.h>
#include <IPAddress.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ==========================================
// WiFi Client Mode
// ==========================================

bool Wifi_connect(
    const char* ssid, const char* password,
    int timeoutSeconds = 10,
    bool silentMode = false,
    const char* staticIP = NULL,
    const char* gateway = NULL,
    const char* subnet = NULL,
    const char* dns1 = NULL,
    const char* dns2 = NULL
);

byte Wifi_checkStatus(bool silentMode = false);

// ==========================================
// WiFi Access Point Mode
// ==========================================

bool Wifi_AP_start(
    const char* ssid, const char* password,
    int channel = 1, bool hidden = false,
    int maxConnection = 32
);
int Wifi_AP_checkStatus(bool silentMode = false);
bool Wifi_AP_stop();

// ==========================================
// MQTT Client
// ==========================================

void Mqtt_setup(const char* server, int port, bool silentMode = false);
void Mqtt_setCallback(void (*callback)(char*, byte*, unsigned int), bool silentMode = false);
bool Mqtt_connect(
    const char* clientId, const char* username = NULL, const char* password = NULL,
    const char* willTopic = NULL, const char* willMessage = NULL,
    bool willRetain = false, bool cleanSession = true, bool silentMode = false
);
bool Mqtt_publish(const char* topic, const char* payload, bool retain = false, bool silentMode = false);
bool Mqtt_subscribe(const char* topic, int qos = 0, bool silentMode = false);
bool Mqtt_unsubscribe(const char* topic, bool silentMode = false);
bool Mqtt_checkStatus(bool silentMode = false);
bool Mqtt_loop();
void Mqtt_disconnect(bool silentMode = false);

// ==========================================
// Bluetooth Classic
// ==========================================

void BT_setup(const char* btName = "ESP32_BT", bool silentMode = false);
void BT_setCallback(void (*callback)(String), bool silentMode = false);
void BT_loop();
bool BT_sendMessage(const String &message, bool ln = true, bool silentMode = false);
bool BT_checkStatus();

// ==========================================
// Bluetooth Low Energy
// ==========================================

// 藍牙回調函式指針類型
typedef void (*BLECallbackFunction)(String);

// BLE 服務和特徵 UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// BLE 初始化
void BLE_setup(const char* bleName, bool silentMode = false);

// 設定接收訊息回調函式
void BLE_setCallback(BLECallbackFunction callback, bool silentMode = false);

// BLE 主迴圈處理
void BLE_loop();

// 發送訊息
bool BLE_sendMessage(const String &message, bool ln = true, bool silentMode = false);

// 檢查連接狀態
bool BLE_checkStatus();

#endif