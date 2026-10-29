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

String msg_recu;
uint8_t Valeur_Aleatoire_Join_Req[16] = {0};

uint8_t Message[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
uint8_t MIC[4];
uint8_t Message_chiffre[16];
uint8_t Message_et_MIC[20];

uint8_t Buffer_Temporaire[16];

uint8_t Tentative=1;
unsigned int TimeOut =5000; 

bool Authentifie = false;

void setup() {
  
  Serial.begin(9600);  
  delay(5000);                  
  Serial.println("Client");

  if (!LoRa.begin(868E6)) 
      Serial.println("Problème d'initialisation du Lora");

  if (myCypher_Primaire.setKey(Cle_Primaire,myCypher_Primaire.keySize()) )
      Serial.println("Clé primaire installé");
}

void loop() {

    while ( (Tentative <=4) and (!Authentifie) ) 
    {
       for (uint8_t i=0; i<16; i++)
                  Valeur_Aleatoire_Join_Req[i] = random(255);

       myCypher_Primaire.encryptBlock(Buffer_Temporaire,Valeur_Aleatoire_Join_Req);
       memcpy(MIC, Buffer_Temporaire , 4);

       memcpy(Message_et_MIC, Valeur_Aleatoire_Join_Req,16);
       memcpy(Message_et_MIC+16, MIC,4);
       
       envoyer_message_au_serveur(Join_Request,Message_et_MIC,20);
      
      bool reponse_recue= false;
      long int start= millis(); 
          do 
          {
              if( LoRa.parsePacket() != 0)      
              {  
                 int dest = LoRa.read();          // Destinatiare
                 byte source = LoRa.read();            // Source
                 byte type_msg_recu = LoRa.read();     // type du message reçu
                 byte long_msg_recu = LoRa.read();    // longueur du message reçu
                 msg_recu = "";
                 while (LoRa.available()) {
                    msg_recu += (char)LoRa.read();
                    }
                    
                 if (long_msg_recu == msg_recu.length() and dest == adresse_client and type_msg_recu == Join_Response) 
                 { 
                  Serial.println ("Tentative " + String(Tentative) + " Réponse à la demande d'authentification reçue: "+ String(msg_recu[0]));
                  reponse_recue=1;
                  Authentifie = (msg_recu[0] !=0);
                 }     
               }
           } while ( !reponse_recue and (millis() - start < TimeOut) );

           if (!reponse_recue)
            Serial.println("Tentative " + String(Tentative) + " Time out, résultat d'authentification non recu");
       
    Tentative = Tentative + 1;
    delay(3000);
    }

    if (Authentifie)

        {
        Serial.println("Bravo, vous êtes authentifié");

        
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

        
        while (Authentifie)

        {
          
          Serial.println ("\n\n Envoi d'un message non chiffre, sans intégrité");
          envoyer_message_au_serveur(Donnee_non_chiffre_non_integ, Message, 16);

          delay(4000);


          Serial.println ("\n\n Envoi d'un message chiffre, sans intégrité \n");
          myCypher_Chiffrement.encryptBlock(Message_chiffre,Message);       
          envoyer_message_au_serveur(Donnee_chiffre_non_integ, Message_chiffre, 16);

          delay(4000);


          Serial.println ("\n\n Envoi d'un message non chiffre, avec intégrité \n");
           
          myCypher_Integrite.encryptBlock(Buffer_Temporaire,Message);
          memcpy(MIC, Buffer_Temporaire , 4);

          memcpy(Message_et_MIC, Message,16);
          memcpy(Message_et_MIC+16, MIC,4);
          envoyer_message_au_serveur(Donnee_non_chiffre_integ, Message_et_MIC, 20);


          delay(4000);

          Serial.println ("\n\n Envoi d'un message chiffre, avec intégrité  \n");

          myCypher_Chiffrement.encryptBlock(Message_chiffre,Message);
          
          myCypher_Integrite.encryptBlock(Buffer_Temporaire,Message_chiffre);
          memcpy(MIC, Buffer_Temporaire , 4);

          memcpy(Message_et_MIC, Message_chiffre,16);
          memcpy(Message_et_MIC+16, MIC,4);
          envoyer_message_au_serveur(Donnee_chiffre_integ, Message_et_MIC, 20);

          delay(4000);
          
        }
        
        }
       
    else
        Serial.println("L'authentification a échoué après 4 tentatives, on réessaie dans 2 minutes");
        
    delay(120000);
    Tentative=1;
}

void envoyer_message_au_serveur(byte type_message, uint8_t * message, uint8_t longeur) {
  LoRa.beginPacket();                   
  LoRa.write(adresse_serveur);              
  LoRa.write(adresse_client);             
  LoRa.write(type_message);             
  LoRa.write(longeur);
  for (uint8_t k=0;k<  longeur;k++)      
      LoRa.write(message[k]);                 
  LoRa.endPacket();                                               
}

void PrintHex8(uint8_t *data, uint8_t length) {
       Serial.print("0x ");
       for (int i=0; i<length; i++) 
           Serial.print(data[i],HEX);
       Serial.print(" \n");       
}
