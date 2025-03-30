#include "Wireless_mgmt.h"
#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>

// MQTT 客戶端實例
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// MQTT 回調函數指針
void (*_mqttCallback)(char*, byte*, unsigned int) = NULL;

/**
 * 設定MQTT連接參數
 * @param server MQTT伺服器地址
 * @param port MQTT伺服器埠，預設為1883
 * @param username MQTT使用者名稱，可為NULL
 * @param password MQTT密碼，可為NULL
 */
void Mqtt_setup(const char* server, int port, bool silentMode) {
  mqttClient.setServer(server, port);
  
  const char* separatorLine = "--------------------------------";
  if (!silentMode) {
    Serial.println(separatorLine);
    Serial.println("MQTT 設定:");
    Serial.print("- 伺服器: ");
    Serial.println(server);
    Serial.print("- 埠: ");
    Serial.println(port);
  }
  
  if (_mqttCallback != NULL) {
    mqttClient.setCallback(_mqttCallback);
    if (!silentMode) {
      Serial.println("- 回調函數已設定");
    }
  }

  if (!silentMode) {
    Serial.println(separatorLine);
  }
}

/**
 * 設定MQTT訊息回調函數
 * @param callback 回調函數指針(void)(char*, byte*, unsigned int)
 */
void Mqtt_setCallback(void (*callback)(char*, byte*, unsigned int), bool silentMode) {
  _mqttCallback = callback;
  mqttClient.setCallback(callback);
  if (!silentMode) {
    Serial.println("MQTT回調函數已設定");
  }
}

/**
 * 連接到MQTT伺服器
 * @param clientId MQTT客戶端ID
 * @param username MQTT使用者名稱，可為NULL
 * @param password MQTT密碼，可為NULL
 * @param willTopic 遺囑主題，可為NULL
 * @param willMessage 遺囑訊息，可為NULL
 * @param willRetain 遺囑是否保留，預設為false
 * @param cleanSession 是否清除會話，預設為true
 * @return 是否成功連接
 */
bool Mqtt_connect(const char* clientId, const char* username, const char* password, 
                 const char* willTopic, const char* willMessage, bool willRetain, bool cleanSession, bool silentMode) {
  if (!silentMode) {
    Serial.println("--------------------------------");
    Serial.print("連接到MQTT伺服器... ");
  }
  
  bool success = false;
  
  if (willTopic != NULL && willMessage != NULL) {
    success = mqttClient.connect(clientId, username, password, willTopic, 0, willRetain, willMessage, cleanSession);
  } else {
    success = mqttClient.connect(clientId, username, password);
  }
  
  if (!silentMode) {
    if (success) {
      Serial.println("成功!");
      Serial.print("- 客戶端ID: ");
      Serial.println(clientId);
      if (username != NULL) {
        Serial.print("- 使用者: ");
        Serial.println(username);
      }
    } else {
      Serial.print("失敗! 錯誤碼: ");
      Serial.println(mqttClient.state());
      Serial.println("嘗試重新連接...");
    }
    Serial.println("--------------------------------");
  }

  return success;
}

/**
 * 發布MQTT訊息
 * @param topic 主題
 * @param payload 訊息內容
 * @param retain 是否保留訊息
 * @return 是否成功發布
 */
bool Mqtt_publish(const char* topic, const char* payload, bool retain, bool silentMode) {
  if (!mqttClient.connected()) {
    if (!silentMode) {
      Serial.println("MQTT未連接，無法發布訊息");
    }
    return false;
  }
  
  bool success = mqttClient.publish(topic, payload, retain);
  
  if (!silentMode) {
    if (success) {
      Serial.print("訊息已發布至主題: ");
      Serial.println(topic);
    } else {
      Serial.print("發布訊息失敗! 主題: ");
      Serial.println(topic);
    }
  }

  return success;
}

/**
 * 訂閱MQTT主題
 * @param topic 主題
 * @param qos 服務品質 (0, 1, 2)
 * @return 是否成功訂閱
 */
bool Mqtt_subscribe(const char* topic, int qos, bool silentMode) {
  if (!mqttClient.connected()) {
    if (!silentMode) {
      Serial.println("MQTT未連接，無法訂閱主題");
    }
    return false;
  }
  
  bool success = mqttClient.subscribe(topic, qos);

  if (!silentMode) {
    if (success) {
      Serial.print("已訂閱主題: ");
      Serial.println(topic);
    } else {
      Serial.print("訂閱主題失敗: ");
      Serial.println(topic);
    }
  }
  
  return success;
}

/**
 * 取消訂閱MQTT主題
 * @param topic 主題
 * @return 是否成功取消訂閱
 */
bool Mqtt_unsubscribe(const char* topic, bool silentMode) {
  if (!mqttClient.connected()) {
    if (!silentMode) {
      Serial.println("MQTT未連接，無法取消訂閱");
    }
    return false;
  }
  
  bool success = mqttClient.unsubscribe(topic);

  if (!silentMode) {
    if (success) {
      Serial.print("已取消訂閱主題: ");
      Serial.println(topic);
    } else {
      Serial.print("取消訂閱失敗: ");
      Serial.println(topic);
    }
  }
  
  return success;
}

/**
 * 檢查MQTT連接狀態
 * @param silentMode 是否靜默模式 (不顯示連線資訊)
 * @return 是否已連接
 */
bool Mqtt_checkStatus(bool silentMode) {
  bool isConnected = mqttClient.connected();
  
  if (!silentMode) {
    Serial.println("----------- MQTT 狀態 -----------");
    Serial.print("連接狀態: ");
    Serial.println(isConnected ? "已連接" : "未連接");
    
    if (!isConnected) {
      Serial.print("錯誤碼: ");
      Serial.println(mqttClient.state());
    }
    Serial.println("--------------------------------");
  }
  
  return isConnected;
}

/**
 * 維持MQTT連線
 * 此函數應該在main loop中定期呼叫
 * @return 目前的連線狀態
 */
bool Mqtt_loop() {
  return mqttClient.loop();
}

/**
 * 斷開MQTT連線
 */
void Mqtt_disconnect(bool silentMode) {
  if (!silentMode) {
    Serial.println("--------------------------------");
    Serial.print("斷開MQTT連線... ");
  }
  mqttClient.disconnect();
  if (!silentMode) {
    Serial.println("已斷開MQTT連線");
    Serial.println("--------------------------------");
  }
}