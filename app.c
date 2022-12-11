#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>


char* getPrice(char[]);
char* parsePrice(int[]);

int main(){
    char symbol[100];
    printf("Please enter the symbol of the coin on which you want predictions: ");
    fgets(symbol, 100, stdin);      //gets the symbol
    symbol[strcspn(symbol, "\n")] = 0;      //removes the escape sequence
    printf("%s",getPrice(symbol));              
    return 0;
}

char* getPrice(char symbol[]){
    char result[300];
    CURL *curl;
    CURLcode response;
    curl_global_init(CURL_GLOBAL_ALL);
    char url[] = "https://pro-api.coinmarketcap.com/v1/cryptocurrency/quotes/latest?symbol=";
    strcat(url,symbol);
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "X-CMC_PRO_API_KEY: d49283ec-042d-49c9-8a38-945fb4d99f98");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        response = curl_easy_perform(curl);

        if(response != CURLE_OK) {
            fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(response));
            return 0;
        } else {
            sprintf(result, "%d", response);
            return result;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

char* parsePrice(int response[]){

}