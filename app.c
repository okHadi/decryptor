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
    int opt;
    char symbol[10];
    char* result;
    float priceInvested;

    menu:
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

    puts("Select an option to work with: ");
    puts(" ");
    sleep(2);
    puts("1- Get prediction calculation");
    puts("2- About us ");
    puts("3- Exit the program");
    puts(" ");
    puts("(Enter 1, 2 or 3.)");

    start:
    scanf("%d", &opt);


    if (opt == 1){
    system("clear");
    sleep(2);
    puts("Welcome to the crypto predictor!");
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


    scanf("%s", symbol);     //gets the symbol
    int symbolCheck = 0;
    FILE* fpointer;
    int buffLength = 255;
    char buff[buffLength]; /* not ISO 90 compatible */
    fpointer = fopen("predictions.txt", "r");
    while(fgets(buff, buffLength, fpointer)) {
            if (strstr(buff, symbol)){
                symbolCheck = 1;
            }
        }
    fclose(fpointer);
    if (symbolCheck == 0){
        sleep(1);
        puts("\nUnfortunately, we could not find the crypto you have mentioned");
        puts("in our database.");
        sleep(1);
        puts(" ");
        puts("Try entering another crypto.");
        sleep(2);
        goto symbol;
    }


    puts(" ");
    sleep(1);


    puts("Enter the price you want to invest: ");
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
        for (int z=7,j=0; z<16; z++, j++)
        {
            if(isdigit(priceStr[z]) || ispunct(priceStr[z]))
            {
            finalPrice[j] = priceStr[z];
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
    
    double num;
    char *token;
    int i= 0;
    int arrlen23 = 0;
    int arrlen24 = 0;
    int arrlen25 = 0;


    if ( file != NULL )
    {
        char line1[1000]; /* or other suitable maximum line size */
        while (fgets(line1, sizeof line1, file) != NULL) /* read a line */
        {
            if (strstr(line1, symbol)){
                check = 1;
                continue;
            }          
            else if (check==1 && !(isalpha(line1[1]))){
                if (count==0){
                    token=strtok(line1, " ");    
                    while (token!= NULL){
                        token=strtok(NULL," ");
                        arrlen23++;

                    }
                    count++;
                    continue;
                }
                else if (count==1){
                    token=strtok(line1, " ");
                    while (token!= NULL){
                        token=strtok(NULL," ");
                        arrlen24++;
                    }
                    count++;
                    continue;
                }
                else if (count==2){
                    token=strtok(line1, " ");
                    while (token!= NULL){
                        token=strtok(NULL," ");
                        arrlen25++;
                    }
                    count++;
                    continue;
                }
            }
            else if(check==1 && isalpha(line1[1])){
                break;
            }
            else{
                continue;
            }
        }
        fclose(file);
    }
    else{
        puts("Error: Could not find the predictions data.");
    }


    //printf("%d", arrlen23);

    FILE *file1 = fopen(filename, "r");
    double price23[arrlen23], price24[arrlen24], price25[arrlen25];
    if ( file != NULL )
    {
        char line[1000]; /* or other suitable maximum line size */
        while (fgets(line, sizeof line, file1) != NULL) /* read a line */
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
                        printf("%d", i);
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
        fclose(file1);
    }
    else{
        puts("Error: Could not find the predictions data.");
    }
    /**********************CALCULATE THE MEAN********************************/
    float avg23 = 0;
    float avg24 = 0;
    float avg25 = 0;

    for (int j=0; j< arrlen23 ; j++){
        //printf("\n%d", j);
        //printf("%f", price23[j]);
        avg23 += price23[j];
    }
    for (int j=0; j< arrlen24 ; j++){
        avg24 += price24[j];
    }
    for (int j=0; j< arrlen25 ; j++){
        avg25 += price25[j];
    }
    //printf("%f \n", avg23);
    //printf("%d", size23);
    avg23 = avg23/arrlen23;
    avg24 = avg24/arrlen24;
    avg25 = avg25/arrlen25;
    //sleep(1);

    puts("According to the predictions, your ROI by 2023 will be:");
    float profit23 = (priceInvested/finPrice)*avg23;
    printf("%lf", profit23);
    }
    }
    else if(opt == 2){
        system("clear");
        int opt2;
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
        puts("(Enter 1 or 2)");
        scanf("%d", &opt2);
        if (opt2==1){
            goto menu;
        }
        else if(opt2 == 2){
            return 0;
        }
        else{
            puts("Please enter a correct option (1 or 2)");
            goto correctOpt;
        }

    }
    else if (opt == 3){
        return 0;
    }
    else{
        puts("Please enter a correct option.");
        goto start;
    }


}
