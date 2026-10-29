#include <SPI.h>
#include <LoRa.h>
#include <Crypto.h>
#include <AES.h>

AES128 myCypher_Primaire ; // pour l'authentification et la génération de clés
AES128 myCypher_Chiffrement ; // pour le chiffrement
AES128 myCypher_Integrite ;  // pour l'intégrité

const byte adresse_client = 0xAA;     // Adresse_Client
const byte adresse_serveur = 0xBB;      // Adresse_Serveur

const byte Join_Request = 0;
const byte Join_Response = 1;
const byte Donnee_chiffre_integ = 2;
const byte Donnee_chiffre_non_integ = 3;
const byte Donnee_non_chiffre_integ = 4;
const byte Donnee_non_chiffre_non_integ = 5;
 
uint8_t Cle_Primaire[] = {14, 25, 45, 21, 0, 91, 2, 128, 129, 50, 250, 149, 43, 80, 99, 81};
uint8_t Cle_Chiffrement [16];
uint8_t Cle_Integrite [16];

uint8_t Buffer_Temporaire[16] ;

uint8_t Valeur_Aleatoire_Join_Req[16];

uint8_t Message_Recu[16];
uint8_t Message_et_MIC_Recu[20];
uint8_t MIC_Recu[4] ;

uint8_t MIC_Calcule[4] ;
uint8_t Message_dechiffre[16];


uint8_t rep[1] ;

void setup() {
  Serial.begin(9600);  
  delay(5000);   
  
  Serial.println("Serveur (Adresse" + String(adresse_serveur) + ")");
  
  if (!LoRa.begin(869E6)) 
        Serial.println("Problème d'initialisation du Lora");
  
  if (myCypher_Primaire.setKey(Cle_Primaire , myCypher_Primaire.keySize()) )
        Serial.println("Clé primaire bien installé \n\n");

}

void loop() {

  if( LoRa.parsePacket() != 0)      
          {  
                 
                 int dest = LoRa.read();          // Destinatiare
                 byte source = LoRa.read();            // Source
                 byte type_msg_recu = LoRa.read();     // type du message reçu
                 byte long_msg_recu = LoRa.read();    // longueur du message reçu

                 if (dest == adresse_serveur and type_msg_recu == Join_Request) 
                 {

                 LoRa.readBytes( Message_et_MIC_Recu, long_msg_recu);

                 Serial.println ("Message de demande d'Authentification recu ");

                 memcpy(Valeur_Aleatoire_Join_Req, Message_et_MIC_Recu, 16 );
                 memcpy(MIC_Recu, Message_et_MIC_Recu+16, 4 );

                 myCypher_Primaire.encryptBlock(Buffer_Temporaire,Valeur_Aleatoire_Join_Req);
                 memcpy(MIC_Calcule , Buffer_Temporaire , 4);
                
                boolean match = true;
                for (int i = 0; i < 4; i++) {
                  if (MIC_Calcule[i] != MIC_Recu[i]) 
                      match = false;
                }
                
                rep[0]= (uint8_t) match;
                
                envoyer_message_au_serveur(Join_Response,rep,1); 

                // Génération de clés de session 

                 // Génération de clés de session 
                // Clé de Chiffrement 
                
                memcpy(Buffer_Temporaire, Valeur_Aleatoire_Join_Req,15);
                memcpy(Buffer_Temporaire+15, &adresse_client,1);
                myCypher_Primaire.encryptBlock(Cle_Chiffrement,Buffer_Temporaire);
                Serial.println("Génération de clé de session de chiffrement: ma clé générée est : ");
                PrintHex8(Cle_Chiffrement,16);   
                
               if (myCypher_Chiffrement.setKey(Cle_Chiffrement,myCypher_Chiffrement.keySize()) )
                        Serial.println("\n Clé de session de chiffrementt bien installée \n ");
                        
        
                // Clé d'intégrité
                
                memcpy(Buffer_Temporaire, Valeur_Aleatoire_Join_Req+1,15);
                memcpy(Buffer_Temporaire+15, &adresse_client,1);
                myCypher_Primaire.encryptBlock(Cle_Integrite,Buffer_Temporaire);
                Serial.println("Génération de clé de session d'intégrité: ma clé générée est : ");
                PrintHex8(Cle_Integrite,16);   
                
               if (myCypher_Integrite.setKey(Cle_Integrite,myCypher_Integrite.keySize()) )
                        Serial.println(" \n Clé de session d'intégrité bien installée \n ");

                 }


                else if (dest == adresse_serveur and type_msg_recu == Donnee_chiffre_non_integ) 
                 { 
                  LoRa.readBytes( Message_Recu, long_msg_recu);
                  Serial.println ("\n\n Message de données chiffrées sans intégrité bien reçues \n Données chiffrées");
                  PrintHex8(Message_Recu,16);

                  Serial.println ("Données déchiffrées");
                  myCypher_Chiffrement.decryptBlock(Message_dechiffre,Message_Recu);
                  PrintHex8(Message_dechiffre,16);
                 }

                 else if (dest == adresse_serveur and type_msg_recu == Donnee_chiffre_integ) 

                 { 

                  LoRa.readBytes( Message_et_MIC_Recu, long_msg_recu);
                   
                  Serial.println ("\n\nMessage de données chiffrées avec intégrité bien reçues \n Données chiffrées et MIC : ");
                  PrintHex8(Message_et_MIC_Recu,20);

                
                  memcpy(Message_Recu , Message_et_MIC_Recu, 16 );
                  memcpy(MIC_Recu, Message_et_MIC_Recu+16, 4 );

                  myCypher_Integrite.encryptBlock(Buffer_Temporaire,Message_Recu);
                  memcpy(MIC_Calcule , Buffer_Temporaire , 4);
                
                  boolean match = true;
                  for (int i = 0; i < 4; i++) {
                    if (MIC_Calcule[i] != MIC_Recu[i]) 
                       match = false;
                   }
                
                  if (!match)
                        Serial.println ("MIC erronée, le paquet est rejeté");
                   else
                   { 
                    Serial.println ("MIC correct Ok ! ");
                 
                    myCypher_Chiffrement.decryptBlock(Message_dechiffre,Message_Recu);
                    Serial.println ("Données déchiffrées");
                    PrintHex8(Message_dechiffre,16);
  
                   }

                 
                 
                 
       }


                 

      }        
                
}

void envoyer_message_au_serveur(byte type_message, uint8_t * message, uint8_t longeur) {
  LoRa.beginPacket();                   
  LoRa.write(adresse_client);              
  LoRa.write(adresse_serveur);             
  LoRa.write(type_message);             
  LoRa.write(longeur);
  for (uint8_t k=0;k<longeur;k++)      
      LoRa.write(message[k]);                 
  LoRa.endPacket();                                               
}

void PrintHex8(uint8_t *data, uint8_t length) {
       Serial.print("0x ");
       for (int i=0; i<length; i++) 
           Serial.print(data[i],HEX);
       Serial.print(" \n");
       
}
