#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"


BLEHIDDevice* hid;
BLECharacteristic* inputMouse;


bool connected = false;

class MyCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer){
    connected = true;
    BLE2902* desc = (BLE2902*)inputMouse->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(true);
  }

  void onDisconnect(BLEServer* pServer){
    connected = false;
    BLE2902* desc = (BLE2902*)inputMouse->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(false);
  }
};



void taskServer(void*){


    BLEDevice::init("Flip-O-Matic");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyCallbacks());

    hid = new BLEHIDDevice(pServer);
    inputMouse = hid->inputReport(1); // <-- input REPORTID from report map

    std::string name = "chegewara";
    hid->manufacturer()->setValue(name);

    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hid->hidInfo(0x00,0x02);

  BLESecurity *pSecurity = new BLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

// http://www.keil.com/forum/15671/usb-mouse-with-scroll-wheel/
// Wheel Mouse - simplified version - 5 button, vertical and horizontal wheel
//
// Input report - 5 bytes
//
//     Byte | D7      D6      D5      D4      D3      D2      D1      D0
//    ------+---------------------------------------------------------------------
//      0   |  0       0       0    Forward  Back    Middle  Right   Left (Buttons)
//      1   |                             X
//      2   |                             Y
//      3   |                       Vertical Wheel
//      4   |                    Horizontal (Tilt) Wheel
//
// Feature report - 1 byte
//
//     Byte | D7      D6      D5      D4   |  D3      D2  |   D1      D0
//    ------+------------------------------+--------------+----------------
//      0   |  0       0       0       0   |  Horizontal  |    Vertical
//                                             (Resolution multiplier)
//
// Reference
//    Wheel.docx in "Enhanced Wheel Support in Windows Vista" on MS WHDC
//    http://www.microsoft.com/whdc/device/input/wheel.mspx
//


const uint8_t reportMapMouse[] = {
0x05, 0x0D,        // Usage Page (Digitizer)
0x09, 0x02,        // Usage (Pen)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0x05, 0x0D,        //   Usage Page (Digitizer)
0x09, 0x20,        //   Usage (Stylus)
0xA1, 0x00,        //   Collection (Physical)
0x09, 0x42,        //     Usage (Tip Switch)
0x09, 0x32,        //     Usage (In Range)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x01,        //     Report Size (1)
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x75, 0x01,        //     Report Size (1)
0x95, 0x06,        //     Report Count (6)
0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
0x09, 0x30,        //     Usage (X)
0x16, 0x00, 0x00,  //     Logical Minimum (0)
0x36, 0x00, 0x00,  //     Physical Minimum (0)
0x26, 0x80, 0x07,  //     Logical Maximum (1920)
0x46, 0x80, 0x07,  //     Physical Maximum (1920)
0x09, 0x31,        //     Usage (Y)
0x16, 0x00, 0x00,  //     Logical Minimum (0)
0x36, 0x00, 0x00,  //     Physical Minimum (0)
0x26, 0x38, 0x04,  //     Logical Maximum (1080)
0x46, 0x38, 0x04,  //     Physical Maximum (1080)
0x75, 0x10,        //     Report Size (16)
0x95, 0x02,        //     Report Count (2)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xC0,              // End Collection

// 72 bytes

};

    hid->reportMap((uint8_t*)reportMapMouse, sizeof(reportMapMouse));
    hid->startServices();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(HID_MOUSE);
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();
    hid->setBatteryLevel(7);

    ESP_LOGD(LOG_TAG, "Advertising started!");
    delay(portMAX_DELAY);
  
};



void setup() {
  
  Serial.begin(115200);
  Serial.println("Starting BLE work!");



  xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
}


void loop() {
  
  if(connected){

    //vTaskDelay(5000);Serial.println("dormindo");

          Serial.println("go to 0xff 0xff");
          //<button>, <x>, <y>, <wheel>

          uint8_t x1 = 0x37;
          uint8_t x2 = 0x04;
          uint8_t y1 = 0x37;
          uint8_t y2 = 0x04;
          
          unsigned char buffer[] = {0x01, 0x00, x1 & 0xFF, (x2 >> 8) & 0xFF, y1 & 0xFF, (y2 >> 8) & 0xFF};
         
          inputMouse->setValue(buffer,sizeof(buffer));
          inputMouse->notify();
          delay(500);
    
          Serial.println("go to origin");
          //<button>, <x>, <y>, <wheel>

          x1 = 0x00;
          x2 = 0x00;
          y1 = 0x00;
          y2 = 0x00;
          uint8_t buffer2[] = {0x01, 0x00, x1 & 0xFF, (x2 >> 8) & 0xFF, y1 & 0xFF, (y2 >> 8) & 0xFF};
         
          inputMouse->setValue(buffer2,sizeof(buffer2));
          inputMouse->notify();
          delay(500);
    
  }
  delay(50);
}
