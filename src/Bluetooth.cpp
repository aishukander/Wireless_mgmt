#include "Wireless_mgmt.h"
#include "Arduino.h"
#include <BluetoothSerial.h> 

BluetoothSerial SerialBT; 

// 藍牙回調函式指針
void (*_btCallback)(String) = NULL;

/**
 * 設定藍牙連接參數
 * @param btName 藍牙顯示名稱
 */
void BT_setup(const char* btName, bool silentMode) {
  SerialBT.begin(btName);
  
  if (!silentMode) {
    Serial.println("--------------------------------");
    Serial.println("藍牙設定:");
    Serial.print("- 設備名稱: ");
    Serial.println(btName);
    
    if (_btCallback != NULL) {
      Serial.println("- 回調函式已設定");
    }
    Serial.println("--------------------------------");
  }
}

/**
 * 設定藍牙訊息回調函式
 * @param callback 回調函式指針 void(String)
 */
void BT_setCallback(void (*callback)(String), bool silentMode) {
  _btCallback = callback;
  if (!silentMode) {
    Serial.println("藍牙回調函式已設定");
  }
}

/**
 * 藍牙主迴圈處理
 * 此函式應在主迴圈中定期呼叫
 */
void BT_loop() {
  if (SerialBT.available()) {
    String message = SerialBT.readString();
    message.trim();
    _btCallback(message);
  }
}

/**
 * 透過藍牙發送訊息
 * @param message 要發送的訊息
 * @return 是否成功發送
 */
bool BT_sendMessage(const String &message, bool ln, bool silentMode) {
  if (SerialBT.connected()) {
    if (ln) {
      SerialBT.println(message);
    } else {
      SerialBT.print(message);
    }
    return true;
  } else {
    if (!silentMode) {
      Serial.println("藍牙未連接，無法發送訊息");
    }
    return false;
  }
}

/**
 * 檢查藍牙連接狀態
 * @return 是否已連接
 */
bool BT_checkStatus() {
  return SerialBT.connected();
}