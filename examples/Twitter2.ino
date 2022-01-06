/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */
#include <stdlib.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "Twitter.h"

const char* ssid     = "<YOUDSSID>";
const char* password = "<YOURPASSWORD>";
const String BEARER_TOKEN = "<YOU BEARER TOKEN>";



WiFiClientSecure client;
Twitter twitter(&client, BEARER_TOKEN);

void displayTweet(Tweet t)
{
  Serial.print("Tweet from ");
  Serial.println(t.author);
  Serial.println(t.text);
  Serial.println("-------------------------------------");
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Initialization complete");
}

int action = 0;
int secCounter = 0;
bool processing = false;
void loop()
{
  secCounter = (secCounter + 1) % 100;
  delay(100);

  Serial.print(processing? "P" : ".");
  if ((secCounter % 10) == 0)
    Serial.println();
    
  if (!processing && (secCounter == 0))
  {
    Serial.println();
    Serial.println("Loading tweets");

    if (action == 0)
    {
      twitter.updateStatus("Hello");
    }
    else if (action == 1)
    {
      twitter.getTweetsByHashTag("starship", 10);
    }
    else
    {
      twitter.getTweetsByAuthor("ElonMusk", 15);
    }

    action = (action + 1) % 3;

    processing = true;
  }

  if (processing)
  {
    twitter.process();
    if (twitter.completed())
    {
      Serial.print("Loading complete ");
      Serial.println(twitter.numTweets());
      for (int i=0; i<twitter.numTweets(); i++)
      {
        displayTweet(twitter.tweetAt(i));
      }

      secCounter = 0;
      processing = false;
    }
  }
}
