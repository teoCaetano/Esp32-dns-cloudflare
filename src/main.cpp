#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "xxx";               // Replace with your network SSID
const char *password = "xxx"; // Replace with your network password

const char *zone_id = "xxx";   // Replace with your Cloudflare zone ID
const char *record_id = "xxx"; // Replace with your Cloudflare record ID
const char *api_mail = "xxx";
const char *api_token = "xxx"; // Replace with your Cloudflare API token
const char *domain = "xxx";                           // Replace with your domain

const char *publicIP ;

void setup()
{
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient httpBin;

    Serial.println("Starting HTTP GET request...");
    httpBin.begin("http://httpbin.org/ip"); // Replace with your URL

    int httpResponseCodeBin = httpBin.GET();

    if (httpResponseCodeBin > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCodeBin);

      String payload = httpBin.getString();
      Serial.println("Response payload:");
      Serial.println(payload);

      // Parse JSON
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      // Check for parsing errors
      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      // Extract value of "origin"
      const char * ipBin = doc["origin"];
      publicIP = ipBin;
      Serial.print("Origin: ");
      Serial.println(publicIP);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCodeBin);
    }
    httpBin.end();

    // Build request URL
    String url = "https://api.cloudflare.com/client/v4/zones/";
    url += zone_id;
    url += "/dns_records/";
    url += record_id;

    // Build request body
    String requestBody = "{\"type\":\"A\",\"name\":\"";
    requestBody += domain;
    requestBody += "\",\"content\":";
    requestBody += "\"";
    requestBody += publicIP;
    requestBody += "\"";
    requestBody += "}";

    Serial.println (requestBody);

    HTTPClient http;

    Serial.println("Starting HTTP PATCH request...");
    http.begin(url); // Specify the URL here
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Auth-Email", api_mail);
    http.addHeader("X-Auth-Key", api_token);

    int httpResponseCode = http.PATCH(requestBody);

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String payload = http.getString();
      Serial.println("Response payload:");
      Serial.println(payload);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }

  delay(60000); // Make the request every minute
}
