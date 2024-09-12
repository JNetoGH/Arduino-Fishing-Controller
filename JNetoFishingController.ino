#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED
#endif

// Set these to your desired credentials.
const char *ssid = "JNeto Arduino";
const char *password = "jnetolindo";

NetworkServer server(80);

// ENCODER CODE =======================
#define outputA 18  // Rotation A (+)
#define outputB 19  // Rotation B (-)
int counter = 0;
int aState;
int aLastState;
// ====================================

void setup() {

  // ENCODER CODE =====================
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  aLastState = digitalRead(outputA);
  // ==================================

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // A valid password must have more than 7 characters.
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

void loop() {

  // ENCODER CODE =====================
  updateEncoderValues();
  // ==================================

  NetworkClient client = server.accept();  // Listen for incoming clients

  if (client) {                     // If you get a client,
    Serial.println("New Client.");  // Print a message out the serial port
    String currentLine = "";        // Make a String to hold incoming data from the client
    while (client.connected()) {    // Loop while the client's connected
      if (client.available()) {     // If there's bytes to read from the client,
        char c = client.read();     // Read a byte, then
        Serial.write(c);            // Print it out the serial monitor
        if (c == '\n') {            // If the byte is a newline character

          // If the current line is blank, you got two newline characters in a row.
          // That's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // Check if the request is for the main page
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // HTML and JavaScript for updating the counter value dynamically
            client.println("<!DOCTYPE html><html>");
            client.println("<head><title>Encoder Counter</title></head>");
            client.println("<body>");
            client.println("<h1>Encoder Counter Value</h1>");
            client.println("<p>Current Count: <span id='value'>0</span></p>");
            client.println("<script>");
            client.println("function updateCounter() {");
            client.println("fetch('/value').then(response => response.text()).then(data => {");
            client.println("document.getElementById('value').innerText = data;");
            client.println("setTimeout(updateCounter, 16);"); // Fetch the counter value every 16 milliseconds
            client.println("});");
            client.println("}");
            client.println("updateCounter();"); // Initial call to start the loop
            client.println("</script>");
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            break;
          } 
          else {  // If you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {  // If you got anything else but a carriage return character,
          currentLine += c;      // Add it to the end of the currentLine
        }

        // Check if the client requests the counter value
        if (currentLine.endsWith("GET /value")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain");
          client.println();
          client.print(counter);  // Send the current counter value
          client.println();
          break;
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);  // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);  // GET /L turns the LED off
        }
      }
    }
    // Close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

// ENCODER CODE ===============================================================
void updateEncoderValues() {
  aState = digitalRead(outputA);  // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState) {
    // If the outputB state is different from the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(outputB) != aState) {
      counter++;
    } 
    else {
      counter--;
    }
    Serial.print("Position: ");
    Serial.println(counter);
  }
  aLastState = aState;  // Updates the previous state of the outputA with the current state
}
// ==============================================================================
