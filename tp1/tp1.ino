
#define ON_BOARD_LED 2

void setup() {
  Serial.begin(115200);

  pinMode(ON_BOARD_LED, OUTPUT);
}

void loop() {
  Serial.println("TEST");
  digitalWrite(ON_BOARD_LED, HIGH);
  delay(1000);
  digitalWrite(ON_BOARD_LED, LOW);
  delay(1000);
}
