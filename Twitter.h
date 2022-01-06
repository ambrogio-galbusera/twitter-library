#ifndef Twitter_h
#define Twitter_h

//#define TWITTER_PRINT_REPLY
//#define TWITTER_PRINT_REQUEST
//#define TWITTER_PRINT_PROCESS

#define TWITTER_REQUEST_TIMEOUT     5000
#define TWITTER_REQUEST_IDLETIME    100

#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

typedef struct 
{
  String id;
  String text;
  String author;
} Tweet;

class Twitter
{
public:
    Twitter(WiFiClientSecure* client, String bearerKey);
    
    void getTweetsByHashTag(String hashTag, int maxTweets);
    void getTweetsByAuthor(String author, int maxTweets);
 
    void process();
    bool completed();
    String lastError();
    
    int numTweets();
    Tweet tweetAt(int idx);

    // The next two methods are work in progress...
    void tweet(String text);
    void updateStatus(String text);

private:
    typedef enum {
        Status_Idle,
        Status_LoadingTweetsByHashTag,
        Status_LoadingTweetsByAuthor,
        Status_LoadingAuthor,
        Status_TweetingAuth,
        Status_Tweeting,
        Status_UpdateStatusAuth,
        Status_UpdatingStatus
    } StatusEnum;    
    
    void getUrl(String url);
    void postUrl(String url, String token, String payload);
    void postAuthUrl(String keySecret);
    
    void getAuthor(String authorId);
  
    bool parse();
    bool parseTweetStep1();
    bool parseTweetStep2();
    bool parseTweetNoAuthor();
    bool processResponse();
    String extractToken(String line);
    
    WiFiClientSecure* _client;
    String _bearerKey;
    bool _connected;
    Tweet* _tweets;
    int _numTweets;
    int _currTweet;
    bool _startFound;
    String _line;
    String _newText;
    unsigned long _requestMillis;
    unsigned long _processMillis;
    DynamicJsonDocument* _doc;
    String _lastError;
    StatusEnum _status;
};

#endif
