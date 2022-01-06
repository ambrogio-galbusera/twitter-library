
#include "Twitter.h"

String KEY_SECRET("<KEY>:<SECRETKEY>");

const char _Base64AlphabetTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

//Private utility functions
inline void base64FromA3ToA4(unsigned char * A4, unsigned char * A3) {
  A4[0] = (A3[0] & 0xfc) >> 2;
  A4[1] = ((A3[0] & 0x03) << 4) + ((A3[1] & 0xf0) >> 4);
  A4[2] = ((A3[1] & 0x0f) << 2) + ((A3[2] & 0xc0) >> 6);
  A4[3] = (A3[2] & 0x3f);
}

inline void base64FromA4ToA3(unsigned char * A3, unsigned char * A4) {
  A3[0] = (A4[0] << 2) + ((A4[1] & 0x30) >> 4);
  A3[1] = ((A4[1] & 0xf) << 4) + ((A4[2] & 0x3c) >> 2);
  A3[2] = ((A4[2] & 0x3) << 6) + A4[3];
}

inline unsigned char base64LookupTable(char c) {
  if(c >='A' && c <='Z') return c - 'A';
  if(c >='a' && c <='z') return c - 71;
  if(c >='0' && c <='9') return c + 4;
  if(c == '+') return 62;
  if(c == '/') return 63;
  return -1;
} 

int base64EncodedLength(int plainLength) {
  int n = plainLength;
  return (n + 2 - ((n + 2) % 3)) / 3 * 4;
} 
int base64Encode(char *output, const char *input, int inputLength) {
  int i = 0, j = 0;
  int encodedLength = 0;
  unsigned char A3[3];
  unsigned char A4[4];

  while(inputLength--) {
    A3[i++] = *(input++);
    if(i == 3) {
      base64FromA3ToA4(A4, A3);

      for(i = 0; i < 4; i++) {
        output[encodedLength++] = _Base64AlphabetTable[A4[i]];
      }

      i = 0;
    }
  }

  if(i) {
    for(j = i; j < 3; j++) {
      A3[j] = '\0';
    }

    base64FromA3ToA4(A4, A3);

    for(j = 0; j < i + 1; j++) {
      output[encodedLength++] = _Base64AlphabetTable[A4[j]];
    }

    while((i++ < 3)) {
      output[encodedLength++] = '=';
    }
  }
  output[encodedLength] = '\0';
  return encodedLength;
}
 

Twitter::Twitter(WiFiClientSecure* client, String bearerKey)
{
    _client = client;
    _bearerKey = bearerKey;
    _status = Status_Idle;
    _tweets = NULL;
    _doc = new DynamicJsonDocument(16384);
    _connected = false;
}

void Twitter::tweet(String text)
{
  _newText = text;
  
  int encodedLength = base64EncodedLength(KEY_SECRET.length());
  char encodedString[encodedLength];
  base64Encode(encodedString, KEY_SECRET.c_str(), KEY_SECRET.length());
  String keySecret(encodedString);

  Serial.print("KeySecret: ");
  Serial.println(keySecret);
  
  postAuthUrl(keySecret);
  _status = Status_TweetingAuth;
}

void Twitter::updateStatus(String text)
{
  _newText = text;

  int encodedLength = base64EncodedLength(KEY_SECRET.length());
  char encodedString[encodedLength];
  base64Encode(encodedString, KEY_SECRET.c_str(), KEY_SECRET.length());
  String keySecret(encodedString);

  Serial.print("KeySecret: ");
  Serial.println(keySecret);
  
  postAuthUrl(keySecret);
  _status = Status_UpdateStatusAuth;
}

void Twitter::getTweetsByHashTag(String hashtag, int maxTweets)
{
  _numTweets = maxTweets;
  String url = "/2/tweets/search/recent?query=%23" + hashtag + "&max_results=" + String(maxTweets) + "&expansions=author_id";

  getUrl(url);
  _status = Status_LoadingTweetsByHashTag;
}

void Twitter::getTweetsByAuthor(String author, int maxTweets)
{
  _numTweets = maxTweets;
  String url = "/2/tweets/search/recent?query=from:" + author + "&max_results=" + String(maxTweets) + "&expansions=author_id";

  getUrl(url);
  _status = Status_LoadingTweetsByAuthor;
}

int Twitter::numTweets()
{
    return _numTweets;
}

Tweet Twitter::tweetAt(int i)
{
    return _tweets[i];
}

void Twitter::getUrl(String url)
{
#ifdef TWITTER_PRINT_REQUEST
    Serial.print("Requesting URL: ");
    Serial.println(url);
#endif
 
    if (!_connected)
    {
        // Use WiFiClient class to create TCP connections
        if (!_client->connect("api.twitter.com", 443)) {
            Serial.println("connection failed");
            return;
        }

        _connected = true;
    }

    _client->println("GET "+ url + " HTTP/1.1");
    _client->println("Host: api.twitter.com");
    _client->println("User-Agent: arduino/1.0.0");
    _client->println("Authorization: Bearer " + _bearerKey);
    _client->println("");
    
    _startFound = false;
    _line = "";
    _requestMillis = millis();  
}

void Twitter::postUrl(String url, String token, String payload)
{
#ifdef TWITTER_PRINT_REQUEST
    Serial.print("Requesting URL: ");
    Serial.println(url);
#endif
 
    if (!_connected)
    {
        // Use WiFiClient class to create TCP connections
        if (!_client->connect("api.twitter.com", 443)) {
            Serial.println("connection failed");
            return;
        }

        _connected = true;
    }

    _client->println("POST "+ url + " HTTP/1.1");
    _client->println("Host: api.twitter.com");
    _client->println("User-Agent: arduino/1.0.0");
    _client->println("Authorization: Bearer " + token);
    _client->println("Content-type: application/json");
    _client->print("Content-Length: ");
    _client->println(payload.length());
    _client->println("");
    _client->println(payload);
    
    _startFound = false;
    _line = "";
    _requestMillis = millis();  
}

void Twitter::postAuthUrl(String keySecret)
{
#ifdef TWITTER_PRINT_REQUEST
    Serial.print("Requesting URL: ");
    Serial.println(url);
#endif
 
    if (!_connected)
    {
        // Use WiFiClient class to create TCP connections
        if (!_client->connect("api.twitter.com", 443)) {
            Serial.println("connection failed");
            return;
        }

        _connected = true;
    }

    String payload =  "{ \"grant_type\": \"client_credentials\" }";
    
    _client->println("POST /oauth2/token?grant_type=client_credentials HTTP/1.1");
    _client->println("Host: api.twitter.com");
    _client->println("Authorization: Basic " + keySecret);
    _client->println("Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8");
    _client->print("Content-Length: ");
    _client->println(payload.length());
    _client->println("");
    _client->println(payload);
    
    _startFound = false;
    _line = "";
    _requestMillis = millis();  
}

bool Twitter::completed()
{
  return (_status == Status_Idle);
}

void Twitter::process()
{
#ifdef TWITTER_PRINT_PROCESS
    Serial.print("Twitter::process: Millis: ");
    Serial.print(millis());
    Serial.print("; request: ");
    Serial.print(_requestMillis);
    Serial.print("; process: ");
    Serial.print(_processMillis);
    Serial.print("; status: ");
    Serial.println(_status);
#endif

    // This will send the request to the server
    if (_client->available() == 0) 
    {
#ifdef TWITTER_PRINT_PROCESS_
        Serial.print("Twitter::process: No Data: ");
        Serial.println(_line);
#endif
        
        if (_line.length() > 0)
        {
            if (millis() - _processMillis > TWITTER_REQUEST_IDLETIME) {
                // process response
#ifdef TWITTER_PRINT_REPLY
                Serial.println("--------------------------");
                Serial.println(_line);
                Serial.println("--------------------------");
#endif

                processResponse();
            }
        }
        else
        {
            if (millis() - _requestMillis > TWITTER_REQUEST_TIMEOUT) {
//#ifdef TWITTER_PRINT_PROCESS
                Serial.println("Twitter::process: Timeout expired");
//#endif
                
                _lastError = "Client timeout";
                _status = Status_Idle;
            }
        }

        return;
    }

    // Read all the lines of the reply from server and print them to Serial
    while(_client->available()) {
#ifdef TWITTER_PRINT_PROCESS_
        Serial.println("Twitter::process: Data available");
#endif
        
        String tmp = _client->readStringUntil('\r');
        //Serial.println(tmp);

        if (!_startFound)
        {
            int idx = 0;
            while ((idx <= 4) && (tmp[idx] != '{'))
                idx ++;
          
            if (tmp[idx] == '{')
                _startFound = true;
        }

        if (_startFound) _line += tmp;
     }

    _processMillis = millis();
}


void Twitter::getAuthor(String authorId)
{
#ifdef TWITTER_PRINT_REQUEST
  Serial.println("Lookp user: " + authorId);
#endif

  String url = "/2/users/"+authorId;
  getUrl(url);
}

bool Twitter::parse()
{
    DeserializationError error = deserializeJson(*_doc, _line);
    if (error != DeserializationError::Ok)
    {
        _lastError = String("Deserialization error: ") + String(error.f_str());
        return true;
    }
    
    if (_tweets != NULL)
    {
        delete [] _tweets;
        _tweets = NULL;
    }
    
    _tweets = new Tweet[_numTweets];
    _currTweet = 0;

    return false;
}

bool Twitter::parseTweetStep1()
{
    JsonArray arr = (*_doc)["data"];
    if (_currTweet >= arr.size())
        return true;
        
    JsonObject obj = arr[_currTweet];
    String authorId = String((const char*)obj["author_id"]);
  
    getAuthor(authorId);
    return false;
}

bool Twitter::parseTweetStep2()
{
    JsonArray arr = (*_doc)["data"];
    if (_currTweet >= arr.size())
        return true;
    
    JsonObject obj = arr[_currTweet];
    String id = String((const char*)obj["id"]);

    DynamicJsonDocument docAuth(16384);
    DeserializationError error = deserializeJson(docAuth, _line);
    if (error != DeserializationError::Ok)
    {
        _lastError = String("Deserialization error: ") + String(error.f_str());
        return true;
    }

    JsonObject user = docAuth["data"];
    String author = String((const char*)user["name"]);
  
    String text = String((const char*)obj["text"]);

    _tweets[_currTweet].id = id;
    _tweets[_currTweet].text = text;
    _tweets[_currTweet].author = author;
    _currTweet ++;

    return false;
}

bool Twitter::parseTweetNoAuthor()
{
    String authorId;
    String author;
    String text;
    
    JsonArray arr = (*_doc)["data"];
    if (_currTweet >= arr.size())
        return true;
    
    JsonObject obj = arr[_currTweet];
    String id = String((const char*)obj["id"]);
  
    text = String((const char*)obj["text"]);
  
    _tweets[_currTweet].id = id;
    _tweets[_currTweet].text = text;
    _tweets[_currTweet].author = "";
    _currTweet ++;

    return false;
}

String Twitter::extractToken(String line)
{
    DynamicJsonDocument docAuth(16384);
    DeserializationError error = deserializeJson(docAuth, line);
    if (error != DeserializationError::Ok)
    {
        _lastError = String("Deserialization error: ") + String(error.f_str());
        return "";
    }
  
    String token = docAuth["access_token"];
    return token;
}

bool Twitter::processResponse()
{
    if (_status == Status_LoadingTweetsByHashTag)
    {
        if (parse())
        {
            _status = Status_Idle;
            return true;
        }
        
        if (parseTweetStep1())
        {
            _status = Status_Idle;
            return true;
        }

        _status = Status_LoadingAuthor;
    }
    else if (_status == Status_LoadingTweetsByAuthor)
    {
        if (parse())
        {
            _status = Status_Idle;
            return true;
        }

        while (!parseTweetNoAuthor())
          ;

        _status = Status_Idle;
    }
    else if (_status == Status_LoadingAuthor)
    {
        if (parseTweetStep2())
        {
            _status = Status_Idle;
            return true;
        }
        
        if (parseTweetStep1())
        {
            _status = Status_Idle;
            return true;
        }

        _status = Status_LoadingAuthor;
    }
    else if (_status == Status_TweetingAuth)
    {
        String token = extractToken(_line);
        if (token.length() == 0)
        {
            _status = Status_Idle;
            return true;
        }

        postUrl("/1.1/tweets", token, "{ \"text\": \"" + _newText + "\" }");
        _status = Status_Tweeting;
    }
    else if (_status == Status_Tweeting)
    {
        Serial.println(_line);

        _status = Status_Idle;
        return true;
    }
    else if (_status == Status_UpdateStatusAuth)
    {
        String token = extractToken(_line);
        if (token.length() == 0)
        {
            _status = Status_Idle;
            return true;
        }
        
        postUrl("/1.1/statuses/update.json", token, "{ \"status\": \"" + _newText + "\" }");
        _status = Status_UpdatingStatus;
    }
    else if (_status == Status_UpdatingStatus)
    {
        Serial.println(_line);

        _status = Status_Idle;
        return true;
    }

    return false;
}
