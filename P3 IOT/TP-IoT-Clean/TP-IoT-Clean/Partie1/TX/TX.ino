#include <SPI.h>
#include <LoRa.h>

int compteur = 1;
byte message_a_envoyer [50]= "Mon cours prefereeeeee 44 " ;
int nb_octets_a_envoyer = 32 ;
unsigned long Temps_debut_tx ;
float Temps_total_tx ;
typedef struct
{
byte destination ;
byte source ;
int num_trame;
float temperature ;
float humidite ;
} Trame_t ;

Trame_t ma_trame;
void setup() {
  Serial.begin(9600); 
  delay(500);

  Serial.println("Je suis l'émetteur");

  if (!LoRa.begin(867500000)) {
    Serial.println("Problème au démarrage du module LoRa!");
  }
  LoRa.setTxPower(10);
  LoRa.enableCrc();
   // Positionner les différents paramètres (puissance d'émission, SpreadingFactor, Bandwidth, CodingRate, etc.)
}

void loop() {

  ma_trame.destination = 1;
  ma_trame.source = 2;
  ma_trame.num_trame++;
  ma_trame.temperature = 25.76;
  ma_trame.humidite = 60.5;  


  Serial.print("Transmission du paquet: " + String(compteur));
 
  LoRa.beginPacket();

  LoRa.write( (byte* )& ma_trame, sizeof(ma_trame) ) ;
    
  bool succes = LoRa.endPacket();

  if (succes)
  {    
    Serial.println("... Succes ...");   
  }
  else
  {
    Serial.println("... Echec ... ");   
  }

  compteur++;
  delay(10000);
}
