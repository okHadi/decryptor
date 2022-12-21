#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINE_LENGTH 1000
#define SIZE 300
//gcc -o hello_mongoc app.c -I/usr/local/include/libbson-1.0-/usr/local/include/libmongoc-1.0 -lmongoc-1.0 -lbson-1.0 -lcurl
struct memory{                  //struct to store the data from curl
    char *memory;
    size_t size;

};

static size_t write_callback(char *contents, size_t size, size_t nmemb, void *userp){
    //The data from main comes in the contents pointer
    size_t realsize = size * nmemb; //memory allocation setup
    struct memory *mem = (struct memory *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL){
        return 0;
    }
    mem->memory = ptr;
    memcpy(&mem->memory[mem->size], contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}


int main(){

    /****************************INPUT FROM USER**********************************/
    char symbol[10];
    char* result;
    printf("Please enter the symbol of the coin on which you want predictions: ");
    fgets(symbol, 100, stdin);      //gets the symbol
    symbol[strcspn(symbol, "\n")] = 0;      //removes the escape sequence

    /****************************REQUEST SENT TO CRYPTO SERVER******************************************************/
    CURL *curl;
    CURLcode response;
    curl_global_init(CURL_GLOBAL_ALL);
    char url[] = "https://pro-api.coinmarketcap.com/v1/cryptocurrency/quotes/latest?symbol=";
    strcat(url,symbol);

    struct memory chnk;
    chnk.memory = NULL;
    chnk.size = 0;              //initializes the memory

    curl = curl_easy_init();
    if(curl)
    {
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "X-CMC_PRO_API_KEY: d49283ec-042d-49c9-8a38-945fb4d99f98");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);      //when curl runs, it will run the callback function for all data that it will receive
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chnk);           //writes the data to chnk by a pointer
        response = curl_easy_perform(curl);

        if(response != CURLE_OK) 
        {
            fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(response));
        }
        else
        {

        /***************************************PRICE EXTRACTION*******************************************************/
            char *priceStr = NULL;
            priceStr = strstr(chnk.memory, "price");
            int index = priceStr-chnk.memory;
            char finalPrice[20];
            for (int i=7,j=0; i<16; i++, j++)
            {
                if(isdigit(priceStr[i]) || ispunct(priceStr[i]))
                {
                finalPrice[j] = priceStr[i];
                }
            }
            double finPrice = atof(finalPrice);
            //printf("%lf", finPrice);
        }
        curl_easy_cleanup(curl);    
    }
    curl_global_cleanup();

    /**************************************PREDICTION EXTRACTION****************************************************/


    static const char filename[] = "predictions.txt";
    FILE *file = fopen(filename, "r");
    int count = 0;
    int check = 0;
    float year23[102], year24[102];
    float num;
    int i= 0; 
    if ( file != NULL )
    {
        char line[1000]; /* or other suitable maximum line size */
        while (fgets(line, sizeof line, file) != NULL) /* read a line */
        {
            if (strstr(line, symbol)){
                check = 1;
                continue;
            }
            
            else if (check==1 && !(isalpha(line[1]))){
                puts(line);
            }
            else{
                break;
            }
            

            

        }
        fclose(file);
    }
    else{
        puts("error");
    }
}

