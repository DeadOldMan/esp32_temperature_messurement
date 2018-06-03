/*
 Name:		Sketch1.ino
 Created:	4/24/2018 8:11:03 AM
 Author:	mueller
*/


/*
*  This sketch demonstrates how to scan WiFi networks.
*  The API is almost the same as with the WiFi Shield library,
*  the most obvious difference being the different file you need to include:
*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Data wire is plugged into GPIO 15 on the ESP32
#define ONE_WIRE_BUS 14
#define TEMPERATURE_PRECISION 12 // Lower resolution

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address


const char* ssid = "CCV Besucher";
const char* password = "bEsUchErnEtz";

WiFiClientSecure client;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP ntpUDP;

// german ntp pool is used with 60 seconds update interval and 2 hours offset
NTPClient timeClient(ntpUDP, "de.pool.ntp.org", 60, 60 * 2);

String translateEncryptionType(wifi_auth_mode_t encryptionType) {

    switch (encryptionType) {
    case (WIFI_AUTH_OPEN) :
        return "Open";
    case (WIFI_AUTH_WEP) :
        return "WEP";
    case (WIFI_AUTH_WPA_PSK) :
        return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK) :
        return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK) :
        return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE) :
        return "WPA2_ENTERPRISE";
    }
}

void scanNetworks() {

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int numberOfNetworks = WiFi.scanNetworks();

    Serial.print("Number of networks found: ");
    Serial.println(numberOfNetworks);

    for (int i = 0; i < numberOfNetworks; i++) {

        Serial.print("Network name: ");
        Serial.println(WiFi.SSID(i));

        Serial.print("Signal strength: ");
        Serial.println(WiFi.RSSI(i));

        Serial.print("MAC address: ");
        Serial.println(WiFi.BSSIDstr(i));

        Serial.print("Encryption type: ");
        String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
        Serial.println(encryptionTypeDescription);
        Serial.println("-----------------------");

    }
}

void connectToNetwork() {
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Establishing connection to WiFi..");
    }

    Serial.println("Connected to network");

}

void sendTemp2Google( char *URLPart)
{
    const char*  server = "docs.google.com";  // Server URL

    // google root certificate authority, to verify the server

    const char* google_root_ca = \
        "-----BEGIN CERTIFICATE-----\n" \
        "MIIDVDCCAjygAwIBAgIDAjRWMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT\n" \
        "MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i\n" \
        "YWwgQ0EwHhcNMDIwNTIxMDQwMDAwWhcNMjIwNTIxMDQwMDAwWjBCMQswCQYDVQQG\n" \
        "EwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMSR2VvVHJ1c3Qg\n" \
        "R2xvYmFsIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2swYYzD9\n" \
        "9BcjGlZ+W988bDjkcbd4kdS8odhM+KhDtgPpTSEHCIjaWC9mOSm9BXiLnTjoBbdq\n" \
        "fnGk5sRgprDvgOSJKA+eJdbtg/OtppHHmMlCGDUUna2YRpIuT8rxh0PBFpVXLVDv\n" \
        "iS2Aelet8u5fa9IAjbkU+BQVNdnARqN7csiRv8lVK83Qlz6cJmTM386DGXHKTubU\n" \
        "1XupGc1V3sjs0l44U+VcT4wt/lAjNvxm5suOpDkZALeVAjmRCw7+OC7RHQWa9k0+\n" \
        "bw8HHa8sHo9gOeL6NlMTOdReJivbPagUvTLrGAMoUgRx5aszPeE4uwc2hGKceeoW\n" \
        "MPRfwCvocWvk+QIDAQABo1MwUTAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTA\n" \
        "ephojYn7qwVkDBF9qn1luMrMTjAfBgNVHSMEGDAWgBTAephojYn7qwVkDBF9qn1l\n" \
        "uMrMTjANBgkqhkiG9w0BAQUFAAOCAQEANeMpauUvXVSOKVCUn5kaFOSPeCpilKIn\n" \
        "Z57QzxpeR+nBsqTP3UEaBU6bS+5Kb1VSsyShNwrrZHYqLizz/Tt1kL/6cdjHPTfS\n" \
        "tQWVYrmm3ok9Nns4d0iXrKYgjy6myQzCsplFAMfOEVEiIuCl6rYVSAlk6l5PdPcF\n" \
        "PseKUgzbFbS9bZvlxrFUaKnjaZC2mqUPuLk/IH2uSrW4nOQdtqvmlKXBx4Ot2/Un\n" \
        "hw4EbNX/3aBd7YdStysVAq45pmp06drE57xNNB6pXE0zX5IJL4hmXXeXxx12E6nV\n" \
        "5fEWCRE11azbJHFwLJhWC9kXtNHjUStedejV0NxPNO3CBWaAocvmMw==\n" \
        "-----END CERTIFICATE-----\n";


    // You can use x.509 client certificates if you want
    //const char* test_client_key = "";   //to verify the client
    //const char* test_client_cert = "";  //to verify the client

    client.setCACert(google_root_ca);
    //client.setCertificate(test_client_key); // for client verification
    //client.setPrivateKey(test_client_cert);	// for client verification

    Serial.println("\nStarting connection to server...");
    if (!client.connect(server, 443))
        Serial.println("Connection failed!");
    else {
        Serial.println("Connected to server!");
        // Make a HTTP request:
        /* The google docs script URL https://script.google.com/macros/s/AKfycbyVRb-pcHXQFMVXtyK9l8PE4Fu--ugJqw5R6TJx/exec */
        char googleScriptURL[512] = { "GET https://script.google.com/macros/s/AKfycbyVRb-pcHXQFMVXtyK9l8PE4Fu--ugJqw5R6TJx/exec" };
        // add the variable part
        strcat(googleScriptURL, URLPart);
        strcat(googleScriptURL, " HTTP/1.0");
        //client.println("GET https://script.google.com/macros/s/AKfycbyVRb-pcHXQFMVXtyK9l8PE4Fu--ugJqw5R6TJx/exec?Temp1=123.21&Temp2=3234.22 HTTP/1.0");
        client.println(googleScriptURL);
        client.println("Host: scripts.google.com");
        client.println("Connection: close");
        client.println();

        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                Serial.println("headers received");
                break;
            }
        }
        // if there are incoming bytes available
        // from the server, read them and print them:
        while (client.available()) {
            char c = client.read();
            //Serial.write(c);
        }

        client.stop();
    }

}

// function to print the temperature for a device
void printTemperature(int i,  DeviceAddress deviceAddress, char* URLPart)
{
    float tempC = sensors.getTempC(deviceAddress);
    Serial.print("Temp C: ");
    Serial.println(tempC);
    if (i == 0)
    {
        // first element with Timestamp
        sprintf(URLPart, "?Timestamp=%lu&Temp1=%f", timeClient.getEpochTime(), tempC);
    }
    else
    {
        // Next elements
        char tempBuf[20];
        sprintf(tempBuf, "&Temp%d=%f", i+1, tempC);
        strcat(URLPart, tempBuf);
    }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}
void setupTempSensors()
{
    // Start up the library
    sensors.begin();

    delay(100);

    // Grab a count of devices on the wire
    numberOfDevices = sensors.getDeviceCount();

    // locate devices on the bus
    Serial.print("Locating devices...");

    Serial.print("Found ");
    Serial.print(numberOfDevices, DEC);
    Serial.println(" devices.");

    // report parasite power requirements
    Serial.print("Parasite power is: ");
    if (sensors.isParasitePowerMode()) Serial.println("ON");
    else Serial.println("OFF");

    // Loop through each device, print out address
    for (int i = 0; i<numberOfDevices; i++)
    {
        // Search the wire for address
        if (sensors.getAddress(tempDeviceAddress, i))
        {
            Serial.print("Found device ");
            Serial.print(i, DEC);
            Serial.print(" with address: ");
            printAddress(tempDeviceAddress);
            Serial.println();

            Serial.print("Setting resolution to ");
            Serial.println(TEMPERATURE_PRECISION, DEC);

            // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
            sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

            Serial.print("Resolution actually set to: ");
            Serial.print(sensors.getResolution(tempDeviceAddress), DEC);
            Serial.println();
        }
        else{
            Serial.print("Found ghost device at ");
            Serial.print(i, DEC);
            Serial.print(" but could not detect address. Check power and cabling");
        }
    }
    delay(5000);
}


void setup() {

    Serial.begin(115200);

    scanNetworks();
    connectToNetwork();

    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());

    timeClient.begin();
    while (timeClient.update() == false)
    {
        Serial.println("NTP Update failed -> try again");
        delay(1000);
    }
    Serial.println(timeClient.getFormattedTime());

    setupTempSensors();

    //WiFi.disconnect(true);
    //Serial.println(WiFi.localIP());

}

void loop(void)
{
    char URLPart[128];
    
    timeClient.update(); // try ntp update 

    Serial.println(timeClient.getFormattedTime());

    // call sensors.requestTemperatures() to issue a global temperature request to all devices on the bus
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("DONE");
    // Loop through each device, print out temperature data
    for(int i=0;i<numberOfDevices; i++)
    {
        // Search the wire for address
        if(sensors.getAddress(tempDeviceAddress, i))
        {
            // Output the device ID
            Serial.print("Temperature for device: ");
            Serial.println(i,DEC);
            // It responds almost immediately. Let's print out the data
            printTemperature(i, tempDeviceAddress, URLPart); // Use a simple function to print out the data
            delay(300);
        } 
        //else ghost device! Check your power requirements and cabling
    }

    sendTemp2Google(URLPart); // now send it to google

    // now wait for 5 Minutes for new temp measurement
    delay(1000 * 60);

}


