#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>

using std::string;
using std::cout;
using std::endl;
using std::strcpy;

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

unsigned const char *hmac_sha1(unsigned const char *key, unsigned const char *data, int key_size,
                               int data_size) {
    // unsigned int i;
    HMAC_CTX ctx;
    unsigned int len;
    unsigned char *out = new unsigned char[20];
    HMAC_Init(&ctx, &key, key_size, EVP_sha1());
    HMAC_Update(&ctx, data, data_size);
    HMAC_Final(&ctx, out, &len);
    /*unsigned char *HMAC(EVP_sha1(), key, sizeof(key),
                    data, sizeof(data), out, sizeof(out));*/
    HMAC_cleanup(&ctx);
    cout << len << endl;
    for (int i = 0; i < 20; i++) {
        printf("%x", out[i]);
    }
    printf("\n");
    return (unsigned const char *)out;
}

/*string bytes_to_string(unsigned const char *src) {
    std::stringstream buff;
    for (int i = 0; i < (int)sizeof(src); i++) {
        buff << std::hex << (int)src[i];
    }
    return buff.str();
}*/

string base64(unsigned const char *data) {
    BIO *b64, *bmem;
    // char message[data.size()];
    // strcpy(message, data.c_str());
    char *buff;
    b64 = BIO_new(BIO_f_base64());               // BIO to perform b64 encode
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);  // No newlines in data
    bmem = BIO_new(BIO_s_mem());                 // BIO to hold result
    BIO_push(b64, bmem);                         // Chains b64 to bmem
    BIO_write(b64, data, 21);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    /*bio = BIO_new_fp(stdout, BIO_NOCLOSE);
    BIO_push(b64, bio);*/
    long len = BIO_get_mem_data(bmem, &buff);

    string ret = string(buff, len - 1);
    while (ret.size() < 28) {
        ret += "=";
    }
    BIO_free_all(b64);
    return ret;
}

int main() {
    string username;
    cout << "Enter Twitter Username: ";
    getline(std::cin, username);
    pair app_info[8] = {pair("screen_name", username),
                        pair("oauth_consumer_key", username),
                        pair("oauth_nonce", gen_alphanum(42)),
                        pair("oauth_signature", ""),
                        pair("oauth_signature_method", "HMAC_SHA1"),
                        pair("oauth_timestamp", std::to_string(time(0))),
                        pair("oauth_token", ""),
                        pair("oauth_version", "1.0")};

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

    pair encode_info[8];
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

    string sign_key;
    temp0 = curl_easy_escape(curl, secrets[0].c_str(), secrets[0].size());
    temp1 = curl_easy_escape(curl, secrets[1].c_str(), secrets[1].size());
    sign_key = string(temp0) + "&" + string(temp1);
    curl_free(temp0);
    curl_free(temp1);

    char key[sign_key.size()];
    char data[out.size()];
    strcpy(key, sign_key.c_str());
    strcpy(data, out.c_str());
    unsigned const char *sha1hash = hmac_sha1(
        (unsigned const char *)key, (unsigned const char *)data, sizeof(key), sizeof(data));
    unsigned const char test[20] = {0xB6, 0x79, 0xC0, 0xAF, 0x18, 0xF4, 0xE9, 0xC5, 0x87, 0xAB,
                                    0x8E, 0x20, 0x0A, 0xCD, 0x4E, 0x48, 0xA9, 0x3F, 0x8C, 0xB6};
    app_info[3].value = base64(sha1hash);  // test); //sha1hash);
    temp0 = curl_easy_escape(curl, app_info[3].value.c_str(), app_info[3].value.size());
    app_info[3].value = string(temp0);
    cout << app_info[3].value << endl;
    string command =
        "curl --get \'" + url + "\' --data \'screen_name=" + app_info[0].value +
        "\' --header \'Authorization: OAuth oauth_consumer_key=\"" + app_info[1].value +
        "\", oauth_nonce=\"" + app_info[2].value + "\", oauth_signature=\"" + app_info[3].value +
        "\", oauth_signature_method=\"" + app_info[4].value + "\", oauth_timestamp=\"" +
        app_info[5].value + "\", oauth_token=\"" + app_info[6].value + "\", oauth_version=\"" +
        app_info[7].value + "\"\' --verbose > lookup.json";
    cout << command << endl;
    std::system(command.c_str());
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
