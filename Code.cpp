#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <U8g2lib.h>
#include <Wire.h>

// ------------------- PIN DEFINITIONS -------------------
#define RADAR_TX_PIN 2
#define RADAR_OUT_PIN 4
#define WIRE_PIN 5
#define BUZZER_PIN 6
#define LED_PIN 13
#define GSM_RX 7
#define GSM_TX 8
#define GPS_RX 9
#define GPS_TX 10
#define SAFE_BUTTON A0

// ------------------- OLED CONFIG (Memory-Safe Version) -------------------
// Use PAGE BUFFER (fits Uno RAM!)
U8G2_SSD1306_128X64_NONAME_1_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// ------------------- OBJECTS -------------------
SoftwareSerial radarSerial(RADAR_TX_PIN, 3);  // Radar serial
SoftwareSerial sim900(GSM_RX, GSM_TX);        // GSM
SoftwareSerial mygps(GPS_RX, GPS_TX);         // GPS
TinyGPSPlus gps;

// ------------------- VARIABLES -------------------
unsigned long lastRead = 0;
unsigned long buttonPressStart = 0;
bool buttonPressed = false;
bool safeMode = false;

long baselineEnergy = 0;
bool calibrated = false;
bool wireBroken = false;
bool alertSent = false;

const int CALIBRATION_SAMPLES = 15;
long personThreshold[6];

const char *phoneNumbers[] = {
  "9609643460",
  "8002213488"
};
const int totalNumbers = 2;

// ------------------- FUNCTION DECLARATIONS -------------------
long readEnergySample();
int estimateHumans(long energy);
void sendCommand(const __FlashStringHelper *cmd);
bool isNetworkRegistered();
void sendSMS(const char *number, const char *text);
void sendSMSToAll(const char *text);
void showMessage(const char *line1, const char *line2);
String getGPSLocation();
void checkSafeButton();

// ------------------- SETUP -------------------
void setup() {
  Serial.begin(9600);
  radarSerial.begin(115200);
  sim900.begin(9600);
  mygps.begin(9600);

  pinMode(RADAR_OUT_PIN, INPUT);
  pinMode(WIRE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SAFE_BUTTON, INPUT_PULLUP);

  // OLED Init
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_5x8_tr);
  showMessage("Smart Band", "Initializing...");

  Serial.println(F("========================================="));
  Serial.println(F(" WOMEN SAFETY BAND - UNO OPTIMIZED VERSION "));
  Serial.println(F("========================================="));

  delay(1500);
  showMessage("Calibration", "Stay away...");

  // --- Radar Calibration ---
  long totalEnergy = 0;
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    if (radarSerial.available()) totalEnergy += readEnergySample();
    delay(250);
  }
  baselineEnergy = totalEnergy / CALIBRATION_SAMPLES;
  personThreshold[1] = baselineEnergy + 2000;
  personThreshold[2] = baselineEnergy + 4000;
  personThreshold[3] = baselineEnergy + 7000;
  personThreshold[4] = baselineEnergy + 10000;
  personThreshold[5] = baselineEnergy + 13000;
  calibrated = true;
  showMessage("Calibration", "Done ✅");

  // --- GSM Init ---
  showMessage("GSM Module", "Connecting...");
  sendCommand(F("AT"));
  sendCommand(F("AT+CMGF=1"));
  sendCommand(F("AT+CSCS=\"GSM\""));
  sendCommand(F("AT+CNMI=1,2,0,0,0"));

  while (!isNetworkRegistered()) {
    showMessage("Network", "📡 Waiting...");
    delay(2500);
  }

  showMessage("System Ready", "✅ Monitoring...");
  Serial.println(F("✅ GSM Ready! System Active."));
}

// ------------------- LOOP -------------------
void loop() {
  checkSafeButton();

  if (safeMode) {
    digitalWrite(LED_PIN, millis() / 1000 % 2);
    showMessage("SAFE MODE", "Alerts OFF");
    delay(800);
    return;
  }

  // ---- Wire Break Detection ----
  int wireState = digitalRead(WIRE_PIN);
  if (wireState == HIGH && !wireBroken) {
    wireBroken = true;
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Serial.println(F("⚠ ALERT: Wire Cut!"));
    showMessage("⚠ ALERT", "Wire Cut!");
    String gpsText = getGPSLocation();
    char alertMsg[120];
    snprintf(alertMsg, sizeof(alertMsg), "🚨 Wire Cut! %s", gpsText.c_str());
    sendSMSToAll(alertMsg);
  } else if (wireState == LOW && wireBroken) {
    wireBroken = false;
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    Serial.println(F("✅ Wire Restored"));
    showMessage("Wire Restored", "OK ✅");
  }

  // ---- Radar Presence ----
  int presence = digitalRead(RADAR_OUT_PIN);
  if (presence == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println(F("👤 Presence Detected"));
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // ---- Human Count ----
  if (millis() - lastRead > 2000) {
    lastRead = millis();
    long energySum = 0;
    while (radarSerial.available()) {
      energySum += radarSerial.read();
    }

    if (calibrated && energySum > baselineEnergy) {
      int humans = estimateHumans(energySum);
      Serial.print(F("👥 Humans: ")); Serial.println(humans);
      showMessage("People Nearby", humans == 1 ? "1 Detected" : "Multiple");
      if (humans >= 3 && !alertSent) {
        String gpsText = getGPSLocation();
        char alertMsg[120];
        snprintf(alertMsg, sizeof(alertMsg), "🚨 %d people! %s", humans, gpsText.c_str());
        sendSMSToAll(alertMsg);
        alertSent = true;
      }
    } else {
      alertSent = false;
    }
  }
}

// ------------------- FUNCTIONS -------------------
void checkSafeButton() {
  int btnState = digitalRead(SAFE_BUTTON);

  if (btnState == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressStart = millis();
  }
  if (btnState == HIGH && buttonPressed) {
    buttonPressed = false;
  }

  if (buttonPressed) {
    unsigned long pressedDuration = millis() - buttonPressStart;

    if (!safeMode && pressedDuration >= 10000) {
      safeMode = true;
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println(F("🔕 SAFE MODE ACTIVATED"));
      showMessage("SAFE MODE", "Band Off");
      sendSMSToAll("🔕 Band Deactivated");
      delay(1000);
    }

    if (safeMode && pressedDuration >= 3000 && pressedDuration < 10000) {
      safeMode = false;
      Serial.println(F("✅ SAFE MODE DEACTIVATED"));
      showMessage("ACTIVE MODE", "Band On");
      sendSMSToAll("✅ Band Reactivated");
      delay(1000);
    }
  }
}

void showMessage(const char *line1, const char *line2) {
  display.firstPage();
  do {
    display.setFont(u8g2_font_5x8_tr);
    display.drawStr(0, 12, line1);
    display.drawStr(0, 30, line2);
  } while (display.nextPage());
  Serial.print(F("[OLED] ")); Serial.print(line1); Serial.print(F(" -> ")); Serial.println(line2);
}

long readEnergySample() {
  long energy = 0;
  int count = 0;
  while (radarSerial.available() && count < 50) {
    energy += radarSerial.read();
    count++;
  }
  return energy;
}

int estimateHumans(long energy) {
  if (energy <= personThreshold[1]) return 1;
  else if (energy <= personThreshold[2]) return 2;
  else if (energy <= personThreshold[3]) return 3;
  else if (energy <= personThreshold[4]) return 4;
  else return 5;
}

void sendCommand(const __FlashStringHelper *cmd) {
  sim900.println(cmd);
  delay(800);
  while (sim900.available()) {
    Serial.write(sim900.read());
  }
}

bool isNetworkRegistered() {
  sim900.println(F("AT+CREG?"));
  delay(1000);
  String response = "";
  while (sim900.available()) {
    response += (char)sim900.read();
  }
  return (response.indexOf(F("+CREG: 0,1")) != -1 || response.indexOf(F("+CREG: 0,5")) != -1);
}

void sendSMS(const char *number, const char *text) {
  sim900.print(F("AT+CMGS=\""));
  sim900.print(number);
  sim900.println(F("\""));
  delay(1000);
  sim900.print(text);
  delay(500);
  sim900.write(26); // Ctrl+Z
  delay(3000);
}

void sendSMSToAll(const char *text) {
  for (int i = 0; i < totalNumbers; i++) {
    Serial.print(F("📨 Sending SMS to "));
    Serial.println(phoneNumbers[i]);
    showMessage("Sending SMS", phoneNumbers[i]);
    sendSMS(phoneNumbers[i], text);
    delay(1500);
  }
  Serial.println(F("✅ All Messages Sent"));
  showMessage("Messages", "✅ Sent");
}

String getGPSLocation() {
  unsigned long start = millis();
  while (millis() - start < 5000) {
    while (mygps.available()) {
      gps.encode(mygps.read());
      if (gps.location.isUpdated()) {
        String lat = String(gps.location.lat(), 6);
        String lng = String(gps.location.lng(), 6);
        String link = "https://maps.google.com/?q=" + lat + "," + lng;
        Serial.print(F("📍 Location: ")); Serial.println(link);
        return link;
      }
    }
  }
  Serial.println(F("📍 GPS: Unavailable"));
  return "Location Unavailable";
}
cat > path/to/newfile.ext <<'EOF'
# paste your code here (replace this line)
EOF

git add path/to/newfile.ext
git commit -m "Add path/to/newfile.ext"
git pushcat > path/to/newfile.ext <<'EOF'
# paste your code here (replace this line)
EOF

git add path/to/newfile.ext
git commit -m "Add path/to/newfile.ext"
git pushcat > path/to/newfile.ext <<'EOF'
# paste your code here (replace this line)
EOF

git add path/to/newfile.ext
git commit -m "Add path/to/newfile.ext"
git pushcat > path/to/newfile.ext <<'EOF'
# paste your code here (replace this line)
EOF

git add path/to/newfile.ext
git commit -m "Add path/to/newfile.ext"
git pushcat > path/to/newfile.ext <<'EOF'
# paste your code here (replace this line)
EOF

git add path/to/newfile.ext
git commit -m "Add path/to/newfile.ext"
git pushcat > path/to/newfile.ext <<'EOF'
# paste your code here (replace this line)
EOF

git add path/to/newfile.ext
git commit -m "Add path/to/newfile.ext"
git push