#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

WiFiManager wm;

AsyncWebServer server(80);

// Informations servant à se connecter au ESP32 la première fois.
const char* ssid = "ESP32";
const char* password = "Patate123";

//Nom du champ.
const char* PARAM_TEST = "inputTest";

// Page HTML qui a seulement un champ texte.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
    
<head>
    <title>Serveur ESP32</title>
    <meta name="viewport" content="width=device-width, initial-scale=1"charset="UTF-8" />
    <script>
      function submitMessage() {
        setTimeout(function(){ document.location.reload(false); }, 500);   
      }
    </script>
</head>

<body>
    <div>
        <form action="/get" target="hidden-form" style="border: 3px groove;padding: 0 20px 20px 20px;width: 350px;margin-bottom:20px;">
            <h2>Prototype du SPIFF</h2>
            <br>
              Champ de test : <input type="text" value="%inputTest%" name="inputTest">
            <br><br><br>
            <input type="submit" value="Appliquer" onclick="submitMessage()">
        </form>
    </div>
    <iframe style="display:none" name="hidden-form"></iframe>
</body>

</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//Fonction qui sert à lire le contenu d'un fichier texte pour aller chercher la valeur d'un champ.
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}

//Fonction qui sert à écrire dans un fichier texte pour enregistrer une valeur.
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Fonction qui remplace les valeurs par défaut dans les champs par les bonnes valeurs enregistrés.
String processor(const String& var){
  //Serial.println(var);
  if(var == "inputTest"){
    return readFile(SPIFFS, "/inputTest.txt");
  }
  return String();
}

void setup() {
  Serial.begin(9600);
  // Initialisation du SPIFFS.
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //Initialisation de WifiManager.
  WiFi.mode(WIFI_STA);
  if(!wm.autoConnect(ssid, password))
		Serial.println("Erreur de connexion.");
	else
		Serial.println("Connexion etablie!");

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Sert à envoyer la page web avec les valeurs enregistrées dans les champs.
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Envoie une requête GET pour enregistrer la valeur du champ.
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputNom
    if (request->hasParam(PARAM_TEST)) {
      inputMessage = request->getParam(PARAM_TEST)->value();
      writeFile(SPIFFS, "/inputTest.txt", inputMessage.c_str());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  //Lecture de la valeur du champ texte.
  String test = readFile(SPIFFS, "/inputTest.txt");

  //Affichage du champ test.
  Serial.println(test);
  
  delay(5000);
}