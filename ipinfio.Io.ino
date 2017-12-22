#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID      = "WIFI_HOME";
const char* WIFI_PASSWORD  = "*";
const char* IPInfoURL    = "https://ipinfo.io/json";

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.print("New Client from IP:");
    Serial.println(client.localIP());

    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then

        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // request ipinfo.io
            DynamicJsonBuffer jsonBuffer;
            HTTPClient http;
            String publicIP = "";
            
            http.begin(IPInfoURL);
            if (http.GET() > 0) {
                  JsonObject& root = jsonBuffer.parseObject(http.getString());
                  String ip = root["ip"];
                  String hostname = root["hostname"];
                  if(hostname.length() < 1){
                    hostname = "No hostname!";
                  }
                  publicIP = ip + " (" + hostname + ")";
            } else {
              publicIP = "Error when try to retrieve";
            }
            http.end();

            client.print("<html>");
              client.print("<head>");
                client.print("<title>IPInfo.io (Arduino Version) v1</title>");
              client.print("<head>");
              client.print("<body style='font-family: sans-serif;'>");
                client.print("<br /><p align='center'>");
                  client.print("<font size='6'>Public IP: ");
                    client.print(publicIP);
                  client.print("</font>");
                  client.print("<br /><font size='4'>Local IP: ");
                    client.print(client.remoteIP());
                  client.print("</font>");
                client.print("</p>");
              client.print("</body>");
            client.print("</html>");

            // The HTTP response ends with another blank line:
            client.println();
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
       
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
