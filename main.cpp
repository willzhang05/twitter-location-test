#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <regex>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <json/json.h>

using std::string;
using std::cout;
using std::endl;
using std::strcpy;
using std::getline;
using std::ifstream;
using std::regex;
using std::regex_replace;

struct attribs {
    string key, value;
    attribs() {}
    attribs(string k, string v) {
        key = k;
        value = v;
    }
    bool operator<(const attribs &other) const {
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

unsigned char *hmac_sha1(unsigned char *key, unsigned char *data, int key_size, int data_size) {
    unsigned char *digest = HMAC(EVP_sha1(), key, key_size, data, data_size, NULL, NULL);
    return digest;
}

string base64(unsigned char *data, int data_size) {
    BIO *b64, *bmem;
    char *buff;
    b64 = BIO_new(BIO_f_base64());               // BIO to perform b64 encode
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);  // No newlines in data
    bmem = BIO_new(BIO_s_mem());                 // BIO to hold result
    BIO_push(b64, bmem);                         // Chains b64 to bmem
    BIO_write(b64, data, data_size + 1);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    long len = BIO_get_mem_data(bmem, &buff);
    string ret = string(buff, len - 1);
    while (ret.size() < (unsigned long)len) {
        ret += "=";
    }
    BIO_free_all(b64);
    return ret;
}

int twitter_lookup(string &username, string &url, string &outfile) {
    attribs app_info[8] = {attribs("screen_name", username),
                           attribs("oauth_consumer_key", ""),
                           attribs("oauth_nonce", gen_alphanum(42)),
                           attribs("oauth_signature_method", "HMAC-SHA1"),
                           attribs("oauth_timestamp", std::to_string(time(0))),
                           attribs("oauth_token", ""),
                           attribs("oauth_version", "1.0"),
                           attribs("oauth_signature", "")};
    string secrets[2], line;
    ifstream infile;
    infile.open("twitter.conf", std::ios::app);
    while (getline(infile, line)) {
        if (line.find("ckey") != string::npos) {
            find_and_replace(line, "ckey=", "");
            app_info[1].value = line;
        } else if (line.find("csecret") != string::npos) {
            find_and_replace(line, "csecret=", "");
            secrets[0] = line;
        } else if (line.find("atoken") != string::npos) {
            find_and_replace(line, "atoken=", "");
            app_info[5].value = line;
        } else if (line.find("asecret") != string::npos) {
            find_and_replace(line, "asecret=", "");
            secrets[1] = line;
        } else {
            continue;
        }
    }
    attribs encode_info[7];
    CURL *curl = curl_easy_init();
    char *temp0;
    for (int i = 0; i < 7; i++) {
        temp0 = curl_easy_escape(curl, app_info[i].key.c_str(), app_info[i].key.size());
        encode_info[i] = attribs(string(temp0), "");
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
    temp0 = curl_easy_escape(curl, url.c_str(), url.size());
    char *temp1 = curl_easy_escape(curl, out.c_str(), out.size());
    out = "GET&" + string(temp0) + "&" + string(temp1);
    curl_free(temp0);
    curl_free(temp1);

    temp0 = curl_easy_escape(curl, secrets[0].c_str(), secrets[0].size());
    temp1 = curl_easy_escape(curl, secrets[1].c_str(), secrets[1].size());
    string sign_key = string(temp0) + "&" + string(temp1);
    curl_free(temp0);
    curl_free(temp1);
    unsigned char *sha1hash = hmac_sha1((unsigned char *)sign_key.c_str(),
                                        (unsigned char *)out.c_str(), sign_key.size(), out.size());
    app_info[7].value = base64(sha1hash, 20);
    temp0 = curl_easy_escape(curl, app_info[7].value.c_str(), app_info[7].value.size());
    app_info[7].value = string(temp0);
    curl_free(temp0);
    //
    string command =
        "curl --get \'" + url + "\' --data \'screen_name=" + app_info[0].value +
        "\' --header \'Authorization: OAuth oauth_consumer_key=\"" + app_info[1].value +
        "\", oauth_nonce=\"" + app_info[2].value + "\", oauth_signature=\"" + app_info[7].value +
        "\", oauth_signature_method=\"" + app_info[3].value + "\", oauth_timestamp=\"" +
        app_info[4].value + "\", oauth_token=\"" + app_info[5].value + "\", oauth_version=\"" +
        app_info[6].value + "\"\' --silent >" + outfile;
    std::system(command.c_str());
    //
    curl_easy_cleanup(curl);
    infile.open(outfile.c_str(), std::ios::app);
    getline(infile, line);
    if (line.find("Bad Authentication Data") != string::npos) {
        return 0;
    }
    return 1;
}

string parse_json(Json::Value &root, string &key) {
    const Json::Value value = root[0][key];
    Json::FastWriter convert;
    return convert.write(value);
}

std::vector<string> get_follower_locations(Json::Value &root) {
    Json::FastWriter convert;
    std::vector<string> ret;
    const Json::Value array = root["users"];
    string buff;
    for (unsigned int i = 0; i < array.size(); i++) {
        buff = convert.write(array[i]["location"]);
        ret.push_back(buff.substr(1, buff.size() - 3));
    }
    return ret;
}

int main() {
    string username;
    cout << "Enter Twitter Username: ";
    getline(std::cin, username);
    string outfile = "lookup.json", url = "https://api.twitter.com/1.1/users/lookup.json";
    if (twitter_lookup(username, url, outfile)) {
        Json::Value user_root;
        ifstream stream;
        stream.open(outfile, ifstream::binary);
        stream >> user_root;
        string query = "location";
        string location = parse_json(user_root, query);
        location = location.substr(1, location.size() - 3);
        query = "time_zone";
        string timezone = parse_json(user_root, query);
        timezone = timezone.substr(1, timezone.size() - 3);
        cout << endl;
        if (location == "" || location == "ul") {
            cout << "Unknown based on location parameter." << endl;
        } else {
            cout << "Known location: " + location << endl;
            return 0;
        }
        if (timezone == "" || timezone == "ul") {
            cout << "Unknown based on timezone parameter." << endl;
        } else {
            cout << "Known timezone: " + timezone << endl;
        }
        stream.close();
        outfile = "list.json";
        url = "https://api.twitter.com/1.1/friends/list.json";
        if (twitter_lookup(username, url, outfile)) {
            Json::Value follower_root;
            stream.open(outfile, ifstream::binary);
            stream >> follower_root;
            stream.close();
            std::vector<string> buff = get_follower_locations(follower_root);
            std::sort(buff.begin(), buff.end());
            buff.resize(std::unique(buff.begin(), buff.end()) - buff.begin());
            if (buff.size() != 0) {
                cout << "Possible locations based on friends:" << endl;
                for (int i = 0; i < (int)buff.size(); i++) {
                    if (buff.at(i) == "") {
                        buff.erase(buff.begin() + i);
                    } else {
                        regex blacklist("([^\u00C0-\u017F\\w\\s,-.])");
                        string out = regex_replace(buff.at(i), blacklist, "");
                        cout << out << endl;
                    }
                }
            }
            return 0;
        }
        return 1;
    }
}
