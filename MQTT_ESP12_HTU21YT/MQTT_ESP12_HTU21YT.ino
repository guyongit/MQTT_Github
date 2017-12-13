/*
Name:		MQTT_ESP12_HTU21
Created:	27/11/2017 12:35:40
Auteur:	limpas guy

Projet objet connecté avec MQTT + Node-RED

*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>			// Librairie associée au HTU21D
#include <SparkFunHTU21D.h> // Librairie capteur HTU21D

#define wifi_ssid "TP-LINK_GUY_LAB" // "Votre Nom_du_routeur"
#define wifi_password "48035866"	// "Votre Mot_de_passe_du_routeur"

#define mqtt_server "192.168.0.101"
// #define mqtt_user "guest"     //s'il a été configuré sur Mosquitto
// #define mqtt_password "guest" //idem

#define temperature_topic "stationEsp12/temperature"  //Topic température
#define humidity_topic "stationEsp12/humidite"        //Topic humidité

//Buffer qui permet de décoder les messages MQTT reçus
char message_buff[100];

long lastMsg = 0;   //Horodatage du dernier message publié sur MQTT
					// long lastRecu = 0;
bool debug = false;  //Affiche sur la console si True

HTU21D htu;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
	Serial.begin(115200);
	setup_wifi();						 //On se connecte au réseau wifi
	client.setServer(mqtt_server, 1883); //Configuration de la connexion au serveur MQTT
	client.setCallback(callback);		//La fonction de callback qui est executée à chaque réception de message   
	htu.begin();  // Initialisation HTU21D
}

//Connexion au réseau WiFi:
void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connexion a ");
	Serial.println(wifi_ssid);

	WiFi.begin(wifi_ssid, wifi_password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Connexion WiFi etablie ");
	Serial.print("=> Addresse IP : ");
	Serial.println(WiFi.localIP());
}

//Reconnexion :
void reconnect() {
	//Boucle jusqu'à obtenur une reconnexion
	while (!client.connected()) {
		Serial.print("Connexion au serveur MQTT...");
		// si MQTT sécurisé alors (client.connect("ESP8266Client", mqtt_user, mqtt_password)).
		if (client.connect("ESP8266Client")) {
			Serial.println("connected");
			//Une fois connecté, on peut s'inscrire à un Topic
			//par exemple : client.subscribe("outTopic/iotTopic")
		}
		else {
			Serial.print("failed, erreur : ");
			Serial.print(client.state());
			Serial.println("5 secondes avant de recommencer");
			delay(5000);
		}
	}
}

void loop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();

	long now = millis();
	//Envoi des mesures toutes les 5000mS
	if (now - lastMsg > 5000) {
		lastMsg = now;
		//Lecture de l'humidité ambiante
		float humd = htu.readHumidity();
		// Lecture de la température en Celcius
		float temp = htu.readTemperature();

		if (debug) {
			Serial.print("Temperature : ");
			Serial.print(temp);
			Serial.print(" | Humidite : ");
			Serial.println(humd);
		}
		//Publication des mesures:
		client.publish(temperature_topic, String(temp).c_str(), true);
		client.publish(humidity_topic, String(humd).c_str(), true);      //et l'humidité
	}
}

// Déclenche les actions à la réception d'un message
void callback(char* topic, byte* payload, unsigned int length) {

	int i = 0;
	if (debug) {
		Serial.println("Message recu =>  topic: " + String(topic));
		Serial.print(" | longueur: " + String(length, DEC));
	}
	/* Lignes de code pour traitement des topics reçus :
	for (i = 0; i<length; i++) {
	message_buff[i] = payload[i];
	}
	message_buff[i] = '\0';

	String msgString = String(message_buff);
	if (debug) {
	Serial.println("Payload: " + msgString);
	}

	if (msgString == "ON") {
	digitalWrite(D2, HIGH);
	}
	else {
	digitalWrite(D2, LOW);
	}
	*/
}
