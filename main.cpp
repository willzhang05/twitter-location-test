#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <openssl/hmac.h>

using std::string;
using std::cout;

struct pair {
    string key, value;
    pair() {}
    pair(string k, string v) {
        key = k;
        value = v;
    }
    bool operator<(const pair &other) const {
        if (key == other.key) {
            return value < other.value;
        }
        return key < other.key;
    }
    string to_string() { return key + "=" + value; }
};

const char ASCII_TABLE[67] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

string find_and_replace(string &s, const string &to_replace, const string &replace_with) {
    return s.replace(s.find(to_replace), to_replace.length(), replace_with);
}

string gen_alphanum(int len) {
    string buff;
    std::srand(time(0));
    for (int i = 0; i < len; i++) {
        buff += ASCII_TABLE[std::rand() % 62];
    }
    return buff;
}

int main() {
    string username;
    cout << "Enter Twitter Username: ";
    getline(std::cin, username);
    ::pair app_info[8] = {::pair("screen_name", username),
                          ::pair("oauth_consumer_key", username),
                          ::pair("oauth_nonce", gen_alphanum(42)),
                          ::pair("oauth_signature", ""),
                          ::pair("oauth_signature_method", "HMAC_SHA1"),
                          ::pair("oauth_timestamp", std::to_string(time(0))),
                          ::pair("oauth_token", ""),
                          ::pair("oauth_version", "1.0")};

    string secrets[2];
    string line;
    std::ifstream infile("twitter.conf");
    while (std::getline(infile, line)) {
        if (line.find("ckey") != string::npos) {
            find_and_replace(line, "ckey=", "");
            app_info[1].value = line;
        } else if (line.find("csecret") != string::npos) {
            find_and_replace(line, "csecret=", "");
            secrets[0] = line;
        } else if (line.find("atoken") != string::npos) {
            find_and_replace(line, "atoken=", "");
            app_info[6].value = line;
        } else if (line.find("asecret") != string::npos) {
            find_and_replace(line, "asecret=", "");
            secrets[1] = line;
        } else {
            continue;
        }
    }

    ::pair encode_info[8];
    CURL *curl = curl_easy_init();
    char *temp0;
    for (int i = 0; i < 8; i++) {
        temp0 = curl_easy_escape(curl, app_info[i].key.c_str(), app_info[i].key.size());
        encode_info[i] = pair(string(temp0), "");
        curl_free(temp0);
        temp0 = curl_easy_escape(curl, app_info[i].value.c_str(), app_info[i].value.size());
        encode_info[i].value = string(temp0);
        curl_free(temp0);
    }
    std::sort(encode_info, encode_info + 7);
    string out = encode_info[0].to_string();
    for (int i = 1; i < 7; i++) {
        out += "&" + encode_info[i].to_string();
    }
    string url = "https://api.twitter.com/1.1/users/lookup.json";
    temp0 = curl_easy_escape(curl, url.c_str(), url.size());
    char *temp1 = curl_easy_escape(curl, out.c_str(), out.size());
    out = "GET&" + string(temp0) + "&" + string(temp1);
    curl_free(temp0);
    curl_free(temp1);
    cout << out + "\n";

    string sign_key;
    temp0 = curl_easy_escape(curl, secrets[0].c_str(), secrets[0].size());
    temp1 = curl_easy_escape(curl, secrets[1].c_str(), secrets[1].size());
    sign_key = string(temp0) + "&" + string(temp1);
    curl_free(temp0);
    curl_free(temp1);
    cout << sign_key + "\n";

    /*string command = "curl --get 'https://api.twitter.com/1.1/users/lookup.json' --data
    'screen_name=" + username + "' --header 'Authorization: OAuth oauth_consumer_key=\"" +
    appinfo[0] + "\", oauth_nonce=\"" + nonce + "\",
    oauth_signature=\"h18hZk1pMPAc8uaxBNLqvc1fQLU\%3D\", oauth_signature_method=\"HMAC-SHA1\",
    oauth_timestamp=\"1457318794\", oauth_token=\"" + appinfo[2] + "\", oauth_version=\"1.0\"'
    --verbose > lookup.json";
    std::system(command.c_str());*/
    /*string url = "https://twitter.com/" + username + "#page-container";
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }*/

    curl_easy_cleanup(curl);
    return 0;
}
