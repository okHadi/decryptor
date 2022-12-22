#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#define MAX_LINE_LENGTH 1000
#define SIZE 300

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
    char opt;
    char symbol[10];
    char* result;
    float priceInvested;
    sleep(2);
    puts("***********************************************************");
    puts("*                                                         *");
    puts("*                                                         *");
    puts("*              Welcome to the DeCryptor                   *");
    puts("*                                                         *");
    puts("*          Decrypt the world of crypto with us            *");
    puts("*                                                         *");
    puts("*                                                         *");
    puts("***********************************************************");
    puts(" ");
    sleep(2);
    start:
    puts("Select an option to work with: ");
    puts(" ");
    sleep(2);
    puts("1- Get prediction calculation");
    puts("2- About us ");
    puts("3- Exit the program");
    puts(" ");
    puts("(Press 1 or 2)");
    opt = getchar();
    if (opt == '1'){
    system("clear");
    sleep(2);
    put("Welcome to the crypto predictor!");
    sleep(2);
    puts(" ");
    puts("To use, first insert the symbo of the coin");
    puts("that you want to get prediction about.");
    puts(" ");
    sleep(1);
    puts("After that, insert the amount you want to invest.");
    puts(" ");
    sleep(2);
    puts("We will show you the ROI according to our predictors.");
    puts(" ");
    puts(" ");
    symbol:
    printf("Enter the symbol of the coin: ");
    fgets(symbol, 10, stdin);      //gets the symbol
    symbol[strcspn(symbol, "\n")] = 0;      //removes the escape sequence
    puts("Enter the pricec you want to invest: ");
    scanf("%f", &priceInvested);

    /****************************REQUEST SENT TO CRYPTO SERVER******************************************************/
    CURL *curl;
    CURLcode response;
    curl_global_init(CURL_GLOBAL_ALL);
    char url[] = "https://pro-api.coinmarketcap.com/v1/cryptocurrency/quotes/latest?symbol=";
    strcat(url,symbol);
    double finPrice;
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
        finPrice = atof(finalPrice);
        //printf("%lf", finPrice);
 
        curl_easy_cleanup(curl);   
    }
    curl_global_cleanup();

    /**************************************PREDICTION EXTRACTION****************************************************/


    static const char filename[] = "predictions.txt";
    FILE *file = fopen(filename, "r");
    int linecount = 0;
    int count = 0;
    int check = 0;
    double price23[500], price24[500], price25[500];
    double num;
    char *token;
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
                if (count==0){
                    token=strtok(line, " ");    
                    while (token!= NULL){
                        price23[i] = atof(token);
                        token=strtok(NULL," ");
                        i++;
                    }
                    count++;
                    i=0;
                    continue;
                }
                else if (count==1){
                    token=strtok(line, " ");
                    while (token!= NULL){
                        price24[i] = atof(token);
                        token=strtok(NULL," ");
                        i++;
                    }
                    count++;
                    i=0;
                    continue;
                }
                else if (count==2){
                    token=strtok(line, " ");
                    while (token!= NULL){
                        price25[i] = atof(token);
                        token=strtok(NULL," ");
                        i++;
                    }
                    count++;
                    continue;
                }
            }
            else if(check==1 && isalpha(line[1])){
                break;
            }
            else{
                continue;;
            }
        }
        fclose(file);
    }
    else{
        puts("Error: Could not find the predictions data.");
    }
    /**********************CALCULATE THE MEAN********************************/
    float avg23 = 0;
    float avg24 = 0;
    float avg25 = 0;
    int size23 = sizeof(price23)/sizeof(price23[0]);
    int size24 = sizeof(price24)/sizeof(price24[0]);
    int size25 = sizeof(price25)/sizeof(price25[0]);
    for (int j=0; j< size23 ; j++){
        avg23 += price23[j];
    }
    for (int j=0; j< size24 ; j++){
        avg24 += price24[j];
    }
    for (int j=0; j< size25 ; j++){
        avg25 += price25[j];
    }
    avg23 = avg23/size23;
    avg24 = avg24/size24;
    avg25 = avg25/size25;
    //printf("%f %f %f", finPrice, priceInvested, avg23);

    puts("According to the predictions, your ROI by 2023 will be:");
    float profit23 = (finPrice/priceInvested)*avg23;
    printf("%lf", profit23);
    }
    }
    else if(opt == '2'){
        char opt2;
        FILE* filePointer;
        int bufferLength = 255;
        char buffer[bufferLength]; /* not ISO 90 compatible */

        filePointer = fopen("aboutus.txt", "r");

        while(fgets(buffer, bufferLength, filePointer)) {
        sleep(1);
        printf("%s\n", buffer);
        }

        fclose(filePointer);

        sleep(5);
        puts("What would you like to do further?");
        sleep(1);
        puts("1- Go to the main menu");
        puts("2- Exit the program");
        correctOpt:
        opt2 = getchar();
        if (opt2=='1'){
            goto start;
        }
        else if(opt2!='1' && opt2!='2'){
            puts("Please enter a correct option (1 or 2)");
            goto correctOpt;
        }

    }

}
