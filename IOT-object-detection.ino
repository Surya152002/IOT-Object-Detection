#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>
 
const char* WIFI_SSID = "ssid";// Enter your id of the camera device of wifi
const char* WIFI_PASS = "password";//Enter the password to connect
 
WebServer server(80);
 
 //Setting the Camera Resoultion 
static auto loRes = esp32cam::Resolution::find(320, 240);//low resoultion
static auto midRes = esp32cam::Resolution::find(350, 530);//medium resoultion
static auto hiRes = esp32cam::Resolution::find(800, 600);//high resoultion
//capturing the images in the format of .jpg 
void serveJpg()
{
  auto frame = esp32cam::capture();
  //Failure Notification condition in which the any obstacles have make while processing
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  //Print statement for the connection between the camera and the ESP32 model
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
 
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  //pointing the frame to the client model to display the captured image
  frame->writeTo(client);
}
  //Funtion for which camera does not support low resoultion 
void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}
  //Funtion for which camera does not support high resoultion 
void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}
 //Funtion for which camera does not support medium resoultion 
void handleJpgMid()
{
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}
 
 
void  setup(){
  Serial.begin(115200);
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);//camera pin
    cfg.setResolution(hiRes);//set the high resolution
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
 
    bool ok = Camera.begin(cfg);//Starting the camera to capture the images
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");//condition for camera condition
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  //check for status of WIFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam-mid.jpg");
 
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam-mid.jpg", handleJpgMid);
 
  server.begin();
}
 
void loop()
{
  server.handleClient();
}
