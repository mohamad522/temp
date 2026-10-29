#include <Arduino.h>
#include <MKRWAN.h>

// OTAA parameters
#define SECRET_APP_EUI "0000000000000000"
#define SECRET_APP_KEY "E438687745E40935CE4950CA25830EAF"

LoRaModem modem;

void setup() {

  Serial.begin(9600);
  delay(3000);

  bool lora_on = modem.begin(EU868);
  if (lora_on)
      Serial.println("Démarrage du module LoRaWAN ... OK");
  else
      Serial.println("Démarrage du module LoRaWAN ... Echec");

  Serial.println(modem.version());
  delay(3000);

  Serial.print("Mon device EUI est: ");
  Serial.println(modem.deviceEUI());
  Serial.flush();

  // OTAA join
  Serial.println("Connexion OTAA en cours...");

  bool connected = modem.joinOTAA(SECRET_APP_EUI, SECRET_APP_KEY);

  if (connected)
  {
    Serial.println("Connexion au réseau LoRaWAN (OTAA) ... OK");

    Serial.println("Mon DevAddr est :");
    Serial.println(modem.getDevAddr());

    // Display session keys
    Serial.println("AppSKey:");
    Serial.println(modem.getAppSKey());

    Serial.println("NwkSKey:");
    Serial.println(modem.getNwkSKey());
  }
  else
  {
    Serial.println("Connexion au réseau LoRaWAN (OTAA) ... ECHEC");
  }
}

void loop() {

  modem.beginPacket();
  modem.print("Message OTAA");
  int err = modem.endPacket(true);

  if (err > 0) {
    Serial.println("Message envoyé correctement");
  } else {
    Serial.println("Erreur d'envoi");
  }

  delay(4000);  // 4 seconds
}
