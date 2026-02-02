#include <WiFi.h>

// Wi-Fi Access Point credentials
const char* ssid = "Opta_WiFi_AP";       // Wi-Fi network name
const char* password = "12345678";       // Wi-Fi network password

// Static IP configuration
IPAddress local_IP(192, 168, 1, 144);    // Static IP address
IPAddress gateway(192, 168, 1, 1);       // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask

WiFiServer server(80);                    // Wi-Fi server on port 80

// Timers
unsigned long Timer1 = 1000;  // Timer 1 (milliseconds)
unsigned long Timer2 = 2000;  // Timer 2 (milliseconds)

// Button pins
#define BUTTON1_PIN A0    // Button1 pin
#define BUTTON2_PIN A1    // Button2 pin
#define WIFI_BUTTON_PIN PE_4  // Wi-Fi button pin (to enable/disable Wi-Fi)

// Wi-Fi state variables
bool wifiEnabled = false;               // Wi-Fi state (enabled or disabled)
unsigned long wifiButtonPressedAt = 0;  // Time when Wi-Fi button was pressed
const unsigned long wifiPressDuration = 1000; // Required press duration to toggle Wi-Fi (1 second)

// Button states
bool button1State = false;             // Button1 state
bool button2State = false;             // Button2 state
bool messageShown = false;             // Flag to avoid showing the message multiple times
bool processRunning = false;           // Flag to know if a process is running
bool ignoreButton1 = false;            // Flag to ignore Button1 during a sequence
bool ignoreButton2 = false;            // Flag to ignore Button2 during a sequence
bool ledState = false;                 // Status LED state (on/off)
unsigned long currentTime = 0;         // Current time (millis)
unsigned long startTime = 0;           // Start time of the sequence
unsigned long lastReadTime = 0;        // Last button read time (for debounce)
unsigned long lastBlinkTime = 0;       // Time of the last LED blink
const unsigned long debounceDelay = 50;  // Debounce delay for buttons
const unsigned long ledInterval = 500;  // LED blink interval in milliseconds

// State machine states
enum State {
  WAITING,     // Waiting state
  FORWARD,     // "Forward" state
  PAUSE_T1,    // Pause T1 (Timer 1)
  ACTIVE_T2,   // State where Timer2 is active
  PAUSE_T3,    // Pause T3 (Timer 1)
  REVERSE      // "Reverse" state
};

State currentState = WAITING;  // Initial state

void setup() {
  // Configure button pins with internal pull-up resistors
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  
  // Relay and LED pins
  pinMode(D0, OUTPUT); // Relay 1
  pinMode(D1, OUTPUT); // Relay 2
  pinMode(LED_D0, OUTPUT);  // LED 1
  pinMode(LED_D1, OUTPUT);  // LED 2
  pinMode(LED_D2, OUTPUT);  // LED 3
  pinMode(LED_D3, OUTPUT);  // LED 4
  pinMode(LED_USER, OUTPUT);  // User LED (Wi-Fi)
  
  // Relays and LEDs (initially off)
  digitalWrite(D0, LOW);
  digitalWrite(D1, LOW);
  digitalWrite(LED_USER, LOW); 
  
  Serial.begin(9600); // Start serial communication
  Serial.println("System ready.");
}

void handleWiFi() {
  static bool previousWifiButtonState = HIGH;              // Previous Wi-Fi button state
  bool currentWifiButtonState = digitalRead(WIFI_BUTTON_PIN); // Current Wi-Fi button state

  if (currentWifiButtonState == LOW && previousWifiButtonState == HIGH) {
    wifiButtonPressedAt = millis(); // Start tracking press time
  } else if (currentWifiButtonState == HIGH && previousWifiButtonState == LOW) {
    // If pressed for at least 1 second, toggle Wi-Fi state
    if (millis() - wifiButtonPressedAt >= wifiPressDuration) {
      wifiEnabled = !wifiEnabled; // Toggle Wi-Fi state

      if (wifiEnabled) {
        WiFi.config(local_IP, gateway, subnet);  // Configure static IP
        if (WiFi.beginAP(ssid, password)) {
          Serial.println("Wi-Fi enabled!");
          Serial.print("IP Address: ");
          Serial.println(WiFi.localIP());        // Show IP address
          server.begin();                        // Start the server
          Serial.println("Server started...");
          digitalWrite(LED_USER, HIGH);          // Turn on user LED
        } else {
          Serial.println("Error enabling Wi-Fi.");
        }
      } else {
        WiFi.disconnect();                       // Disconnect Access Point
        Serial.println("Wi-Fi disabled!");
        digitalWrite(LED_USER, LOW);             // Turn off user LED
      }
    }
  }

  previousWifiButtonState = currentWifiButtonState;

  // Blink the user LED if Wi-Fi is disabled
  if (!wifiEnabled) {
    if (millis() - lastBlinkTime >= ledInterval) {
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(LED_USER, ledState); // Toggle LED
    }
  } else {
    digitalWrite(LED_USER, HIGH); // Keep LED on when Wi-Fi is enabled
  }
}

void loop() {
  
  handleWiFi(); // Handle Wi-Fi button

  // Handle HTTP client connection (web server)
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client Connected.");
    String request = client.readStringUntil('\r');
    Serial.println("Client Request: ");
    Serial.println(request);

    // Update timers
    if (request.indexOf("Timer1=") != -1) {
      int startIndex = request.indexOf("Timer1=") + 7;
      int endIndex = request.indexOf("&", startIndex);
      Timer1 = request.substring(startIndex, endIndex).toInt();
      Serial.print("Timer1 updated: ");
      Serial.println(Timer1);
    }

    if (request.indexOf("Timer2=") != -1) {
      int startIndex = request.indexOf("Timer2=") + 7;
      int endIndex = request.indexOf(" ", startIndex);
      Timer2 = request.substring(startIndex, endIndex).toInt();
      Serial.print("Timer2 updated: ");
      Serial.println(Timer2);
    }

    // HTML response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<h1>Timer Programming</h1>");
    client.println("<form method='GET'>");
    client.println("Timer1 (ms): <input type='number' name='Timer1' value='" + String(Timer1) + "'><br>");
    client.println("Timer2 (ms): <input type='number' name='Timer2' value='" + String(Timer2) + "'><br>");
    client.println("<input type='submit' value='Update'>");
    client.println("</form>");
    client.println("</html>");
    client.println();
    client.stop(); // Close client connection
    Serial.println("Client disconnected.");
  }

  // Handle user LED blinking
  unsigned long currentTime = millis();
  if (currentTime - lastBlinkTime >= ledInterval) {
    lastBlinkTime = currentTime;
    ledState = !ledState;
    digitalWrite(LED_USER, ledState); // Blink the LED
  }

  // Handle button 1 with debounce
  bool currentButton1Read = digitalRead(BUTTON1_PIN) == LOW; // Button pressed = LOW
  if (currentButton1Read != button1State && millis() - lastReadTime > debounceDelay) {
    lastReadTime = millis();
    button1State = currentButton1Read;
  }

  // Handle button 2 with debounce
  bool currentButton2Read = digitalRead(BUTTON2_PIN) == LOW; // Button pressed = LOW
  if (!ignoreButton2 && currentButton2Read != button2State && millis() - lastReadTime > debounceDelay) {
    lastReadTime = millis();
    button2State = currentButton2Read;
  }

  // State machine logic
  switch (currentState) {
    case WAITING:
      digitalWrite(D0, LOW);
      digitalWrite(D1, LOW);
      digitalWrite(LED_D0, HIGH);
      digitalWrite(LED_D1, LOW);
      digitalWrite(LED_D2, LOW);
      digitalWrite(LED_D3, LOW);
      if (button1State) {
        currentState = FORWARD;
        ignoreButton2 = true;
      }
      break;

    case FORWARD:
      digitalWrite(D0, HIGH);
      digitalWrite(D1, LOW);
      digitalWrite(LED_D0, LOW);
      digitalWrite(LED_D1, HIGH);
      digitalWrite(LED_D2, LOW);
      digitalWrite(LED_D3, LOW);
      if (button1State && !messageShown) {
        Serial.println("Machine moving");
        messageShown = true;
      } else if (!button1State) {
        messageShown = false;
        startTime = millis();
        currentState = PAUSE_T1;
        Serial.println("Pause T1 started");
        ignoreButton2 = true;
      }
      break;

    case PAUSE_T1:
      digitalWrite(D0, LOW);
      digitalWrite(D1, LOW);
      digitalWrite(LED_D0, LOW);
      digitalWrite(LED_D1, LOW);
      digitalWrite(LED_D2, HIGH);
      digitalWrite(LED_D3, LOW);
      if (millis() - startTime >= Timer1) {
        currentState = ACTIVE_T2;
        Serial.println("Pause T1 ended, activating Timer2");
        startTime = millis();
      }
      break;

    case ACTIVE_T2:
      digitalWrite(D0, LOW);
      digitalWrite(D1, HIGH);
      digitalWrite(LED_D0, LOW);
      digitalWrite(LED_D1, LOW);
      digitalWrite(LED_D2, LOW);
      digitalWrite(LED_D3, HIGH);
      if (millis() - startTime >= Timer2) {
        currentState = PAUSE_T3;
        Serial.println("Button2 disabled, starting Pause T3");
        startTime = millis();
      }
      break;

    case PAUSE_T3:
      digitalWrite(D0, LOW);
      digitalWrite(D1, LOW);
      digitalWrite(LED_D0, LOW);
      digitalWrite(LED_D1, LOW);
      digitalWrite(LED_D2, HIGH);
      digitalWrite(LED_D3, LOW);
      if (millis() - startTime >= Timer1) {
        currentState = WAITING;
        Serial.println("Pause T3 ended, state WAITING");
        ignoreButton1 = false;
        ignoreButton2 = false;
      }
      break;

    case REVERSE:
      digitalWrite(D0, LOW);
      digitalWrite(D1, HIGH);
      digitalWrite(LED_D0, LOW);
      digitalWrite(LED_D1, LOW);
      digitalWrite(LED_D2, LOW);
      digitalWrite(LED_D3, HIGH);
      if (!button2State) {
        currentState = PAUSE_T3;
        ignoreButton1 = true;
        startTime = millis();
        Serial.println("State PAUSE_T3 started.");
      }
      break;
  }

  // Handle Button2
  if (button2State && currentState != REVERSE) {
    currentState = REVERSE;
    processRunning = true;
    ignoreButton1 = true;
    Serial.println("Reverse mode activated.");
  }
}

