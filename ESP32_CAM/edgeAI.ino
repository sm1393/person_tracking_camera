#include "esp_camera.h"
#include <WiFi.h>
#include <ESP32Servo.h>
#include<iostream>

using namespace std;


//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

extern int xnose;
extern int ynose;

const char* ssid = "iphone";
const char* password = "rfid1234";

Servo myservo1;  // create servo object to control a servo
Servo myservo2;  // create servo object to control a servo


int pos = 0;    // variable to store the servo position


void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  myservo1.setPeriodHertz(50);    // standard 50 hz servo
	//myservo2.attach(2, 500, 2500); // attaches the servo on pin 18 to the servo object

  myservo1.attach(14, 500, 2500);

  //myservo1.write(90);


  myservo2.setPeriodHertz(50);    // standard 50 hz servo
	//myservo2.attach(2, 500, 2500); // attaches the servo on pin 18 to the servo object

  myservo2.attach(15, 500, 2500);

  //myservo2.write(70);
  

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_2;
  config.ledc_timer = LEDC_TIMER_2;
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
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

float error_x,angle_x=90,delangle_x;
float error_y,angle_y=70,delangle_y;


//for x
float Kp_x = 0.08; // Proportional gain
float Ki_x = 0; // Integral gain
float Kd_x = 0.01; // Derivative gain

//for y
float Kp_y = 0.09; // Proportional gain
float Ki_y = 0; // Integral gain
float Kd_y = 0.01; // Derivative gain

float prevError_x = 0;
float integral_x = 0;

float prevError_y = 0;
float integral_y = 0;


void loop() {

  error_x= xnose- 80;

  if (abs(error_x)>3)
  {
    //PID controller
  delangle_x = Kp_x * error_x + Ki_x * integral_x + Kd_x * (error_x - prevError_x);
  angle_x=angle_x+delangle_x;

//Edge Cases
  if (angle_x<30)
  {
    angle_x=90;
  }

  if (angle_x>150)
  {
    angle_x=90;
  }

  //cout<<"error: "<<error_x<<" delangle: "<<delangle_x<<" angle: "<<angle_x<<endl;
  

  if (angle_x>=30 && angle_x<=150)
  {
    myservo1.write(angle_x);
  }
  }
  // Update integral term and previous error for X
  integral_x += error_x;
  prevError_x = error_x;



  ////////////////////////Y-Axis Tracking/////////////////////////////////////////
  error_y= ynose- 60;

  if (abs(error_y)>3)
  {
    //PID controller
  delangle_y = Kp_y * error_y + Ki_y * integral_y + Kd_y * (error_y - prevError_y);
  angle_y=angle_y+delangle_y;

//Edge Cases
  if (angle_y<40)
  {
    angle_y=70;
  }

  if (angle_y>100)
  {
    angle_y=70;
  }

  //cout<<"error: "<<error_y<<" delangle: "<<delangle_y<<" angle: "<<angle_y<<endl;
  

  if (angle_y>=40 && angle_y<=100)
  {
    myservo2.write(angle_y);
  }
  }

  // Update integral term and previous error for Y
  integral_y += error_y;
  prevError_y = error_y;

  delay(100);
}
