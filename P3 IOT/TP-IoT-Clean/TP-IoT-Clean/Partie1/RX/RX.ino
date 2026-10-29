#include <SPI.h>
#include <LoRa.h>

int taille_paquet_recu;

void setup() {
  Serial.begin(9600);

  delay(10000);

  Serial.println("Je suis le récepteur");

  if (!LoRa.begin(868000000)) {
    Serial.println("Problème au démarrage du module LoRa!");
  }

  // Positionner les différents paramètres (SpreadingFactor, Bandwidth, CodingRate, etc.)

}

void loop() {

  taille_paquet_recu = LoRa.parsePacket();
  // Vérifier si un paquet est bien reçu
  
  if (taille_paquet_recu) 
  {
    Serial.print("paquet reçu ' ");

    // Lire et afficher le contenu du paquet reçu octet par octet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // Aficher la puissance du signal reçu
    Serial.print("' avec une puissance du signal (RSSI) égale à: ");
    Serial.println(LoRa.packetRssi());
  }
}
