#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#define MAX_LINE_LENGTH 1000
#define SIZE 300

void adminPanelMenu();
void adminPanel();
void adminPanelOptions();
void mainMenu();
void mainMenuOptions();
void cryptoPredictor();
void wrongSymbolError();
void predictionResult(float, float);
void predictionsLoader();
void aboutusMenu();

struct memory{                  //struct to store the data from curl
    char *memory;
    size_t size;

};

static size_t write_callback(char *contents, size_t size, size_t nmemb, void *userp){

    //The data from main comes in the contents pointer

    size_t realsize = size * nmemb;             //memory allocation setup
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
    /***********************Start database connection******************************/
    // Declare a pointer to the database connection
    sqlite3 *db;

    // Declare a pointer to the error message
    char *err_msg = 0;

    // Open the database connection
    int rc = sqlite3_open("cryptos.db", &db);

    // Check if the connection was successful
    if (rc != SQLITE_OK) {
        // Print an error message and close the connection
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    /****************************INPUT FROM USER**********************************/
    menu:
    puts("");
    mainMenu();
    int opt;
    char symbol[10];
    char* result;
    float priceInvested;
   scanf("%d", &opt);

    if (opt == 1){              //user chooses to get the prediction calculator
    cryptoPredictor();
    symbol:
    printf("Enter the symbol of the coin: ");


    scanf("%s", symbol);     //gets the symbol from the user
    for (int i=0; i<strlen(symbol); i++){       //upper case the symbol
        symbol[i] = toupper(symbol[i]);
    }

    int symbolCheck = 0;
    FILE* fpointer;
    int buffLength = 255;
    char buff[buffLength]; 
    fpointer = fopen("predictions.txt", "r");           //opens the predictions data
    while(fgets(buff, buffLength, fpointer)) {          //to see if the crypto exists 
            if (strstr(buff, symbol)){
                symbolCheck = 1;
            }
        }
    fclose(fpointer);
    if (symbolCheck == 0){                  //if it does not exit, have the user enter another one/enter it again
        wrongSymbolError();
        goto symbol;
    }


    puts(" ");
    sleep(1);


    puts("Enter the amount you want to invest: ");       //ask the user to enter the price he wants to invest
    scanf("%f", &priceInvested);                            //we will give him the ROI according to the average of all predictions

    /****************************REQUEST SENT TO CRYPTO SERVER******************************************************/
    CURL *curl;
    CURLcode response;
    curl_global_init(CURL_GLOBAL_ALL);
    char url[] = "https://pro-api.coinmarketcap.com/v1/cryptocurrency/quotes/latest?symbol=";       //using coinmarketcap api
    strcat(url,symbol);                         //appends the symbol at the end of the api
    double finPrice;
    struct memory chnk;
    chnk.memory = NULL;
    chnk.size = 0;              //initializes the memory


    curl = curl_easy_init();
    if(curl)
    {
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "X-CMC_PRO_API_KEY: d49283ec-042d-49c9-8a38-945fb4d99f98");              //append the key to the header
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);      //when curl runs, it will run the callback function for all data that it will receive
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chnk);           //writes the data to chnk by a pointer
        response = curl_easy_perform(curl);

        if(response != CURLE_OK) 
        {
            fprintf(stderr, "Request to fetch crypto price failed: %s\n", curl_easy_strerror(response));
            mainMenuOptions();
            goto menu;
        }
        else
        {

    /***************************************PRICE EXTRACTION*******************************************************/
        char *priceStr = NULL;
        priceStr = strstr(chnk.memory, "price");            //cuts the string, pointer starts from the "price" part
        char finalPrice[20];                    //string where the price will be stored without any other part of the res data
        for (int z=7,j=0; z<16; z++, j++)
        {
            if(isdigit(priceStr[z]) || ispunct(priceStr[z]))        //we only get the number and the decimal
            {
            finalPrice[j] = priceStr[z];
            }
        }
        finPrice = atof(finalPrice);        //converts the string to a float
 
        curl_easy_cleanup(curl);                //ends the curl call and cleansup the memory
    }
    curl_global_cleanup();

    /**************************************PREDICTION EXTRACTION****************************************************/


    static const char filename[] = "predictions.txt";
    FILE *file = fopen(filename, "r");
    //int linecount = 0;
    int count = 0;
    int check = 0;
    
    double num;
    char *token;
    int i= 0;
    int arrlen23 = 0;
    int arrlen24 = 0;
    int arrlen25 = 0;


    if ( file != NULL )                                             //determines the number of predictions for each year of a crypto
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



    check = 0;
    count = 0;
    i = 0;

    FILE *file1 = fopen(filename, "r");
    float price23[arrlen23], price24[arrlen24], price25[arrlen25];      //assign the array lenght according to the number of predictions
    if ( file != NULL )
    {
        char line[1000];
        while (fgets(line, sizeof line, file1) != NULL) 
        {
            if (strstr(line, symbol)){                          //appends the predictions as floating values
                check = 1;                                      //according to their year in the above declared arrays
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
        avg23 += price23[j];
    }
    for (int j=0; j< arrlen24 ; j++){
        avg24 += price24[j];
    }
    for (int j=0; j< arrlen25 ; j++){
        avg25 += price25[j];
    }
    avg23 = avg23/arrlen23;
    avg24 = avg24/arrlen24;
    avg25 = avg25/arrlen25;
    predictionsLoader();
    float profit23 = (priceInvested/finPrice)*avg23;
    float profit24 = (priceInvested/finPrice)*avg24;
    float profit25 = (priceInvested/finPrice)*avg25;
    char *status;
    puts(" ");
    predictionResult(priceInvested, profit23);
    predictionResult(priceInvested, profit24);
    predictionResult(priceInvested, profit25);
    }
    }


    else if(opt == 2){              //user chooses to read the aboutus section on menu
        system("clear");
        int opt2;
        FILE* filePointer;
        int bufferLength = 255;
        char buffer[bufferLength];

        filePointer = fopen("aboutus.txt", "r");            //prints the about us file

        while(fgets(buffer, bufferLength, filePointer)) {
        sleep(1);
        printf("%s\n", buffer);
        }

        fclose(filePointer);

        aboutusMenu();
        correctOpt:
        scanf("%d", &opt2);
        if (opt2==1){           //goes from about us section to the menu
            mainMenu();
            goto menu;
        }
        else if(opt2 == 2){         //exits the program from the about us sectoin
            return 0;
        }
        else{
            puts("Please enter a correct option (1 or 2)");     //incase user chooses wrong option from about us
            goto correctOpt;
        }

    }
    else if(opt ==3){
        adminPanelMenu();

        char password[100];
        char pass_str[] = "ecef7b1e64c70decb9786df778d470f7288c02eeb6b95c97dade5b46d768ab50";    //temppassword
        unsigned char pass_hash[SHA256_DIGEST_LENGTH];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sscanf(pass_str + (i * 2), "%2hhx", &pass_hash[i]);
        }
        password:
        puts("");
        scanf("%s", password);
        if (strcmp(password, "-1") == 0){
            goto menu;
            //break;
        }
        else{
            unsigned char entered_hash[SHA256_DIGEST_LENGTH];       //encrypts the entered password
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, password, strlen(password));
            SHA256_Final(entered_hash, &sha256);

            if (memcmp(pass_hash, entered_hash, SHA256_DIGEST_LENGTH) == 0) {        //compares the entered password with the original password
                sleep(0.5);
                puts("The password is correct, logging you in...");
                adminPanel();
                adminPanelOptions();
                int adminPanelOpt;
                scanf("%d", &adminPanelOpt);
                if (adminPanelOpt == 1){            //add crypto predictions
                    char *sql = "INSERT INTO cryptos (id, year, prediction) VALUES (?, ?, ?);";
                    // Declare a pointer to the prepared statement
                    sqlite3_stmt *stmt;
  
                    // Prepare the statement
                    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

                    // Check if the statement was prepared successfully
                    if (rc != SQLITE_OK ) {
                        // Print an error message and close the connection
                        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));

                        sqlite3_close(db);

                        return 1;
                    }
                    puts("Enter the id of the coin (e.g BTC)");
                    char idadd[10];
                    scanf("%s", idadd);
                    puts("Enter the year of the prediction between 2023 and 2025");
                    int yearadd;
                    scanf("%d", &yearadd);
                    puts("Enter the prediction:");
                    float predadd;
                    scanf("%f", &predadd);
                    // Bind the values to the placeholders
                    sqlite3_bind_text(stmt, 1, idadd, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 2, yearadd);
                    sqlite3_bind_int(stmt, 3, predadd);
                    rc = sqlite3_step(stmt);

                    if (rc != SQLITE_DONE ) {
                    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return 1;
                    }
                    sleep(1);
                    puts("Added succesfully. What would you like to do?");
                    puts("1- Add another prediction");
                    puts("2- Exit to main menu");

                    sqlite3_finalize(stmt);
                    sqlite3_close(db);

                }

            } 
            else {
                sleep(1);
                puts("The entered password is incorrect");
                puts("Try again.\n");
                goto password;
        }
        }

    }
    else if (opt == 4){
        return 0;                                    //user chooses to exit the program from the menu
    }
    else{                                           //incase user inputs wrong option on the menu
        puts("Please enter a correct option.\n");
        mainMenuOptions();
        goto menu;
    }


}


void predictionResult(float priceInvested, float profityear){
    if (profityear>priceInvested){
        printf("By 2023: %.2f increased to %.2f USD \n", priceInvested, profityear);
        printf("Percetage increase: %.2f\n", ((profityear-priceInvested)/priceInvested)*100);
    }
    else{
        printf("By 2023: %.2f decreased to %.2f USD \n", priceInvested, profityear);
        printf("Percetage decrease: %.2f\n", ((profityear-priceInvested)/priceInvested)*100);
    }
    puts(" ");
    sleep(2);
}


void adminPanelMenu(){
    system("clear");
    sleep(1);
    puts("Welcome to the admin panel.");
    sleep(1);
    puts("To get started, first enter your password: ");
    puts("(If you want to exit, enter -1 in the password field.)");
    puts("Password:");
}


void adminPanel(){
    sleep(1);
    system("clear");
    sleep(1);
    puts("Welcome, admin");
    sleep(1);
    puts("Here you can add your crypto predictions.");
}

void adminPanelOptions(){
    sleep(1);
    puts("Select one of the following options:");
    puts("1- Insert crypto data:");
    puts("2- View crypto data:");
    puts("3- Exit to main menu");
}

void mainMenu(){
    system("clear");
    sleep(1.5);
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
    sleep(1.5);
    mainMenuOptions();
}

void mainMenuOptions(){
    puts("Select an option to work with: ");
    puts(" ");
    sleep(1.5);
    puts("1- Get prediction calculation");
    puts("2- About us ");
    puts("3- Enter the admin panel");
    puts("4- Exit the program");
    puts(" ");
    puts("(Enter 1, 2 or 3 or 4)");
}

void aboutusMenu(){
    sleep(2);
    puts("What would you like to do further?");
    sleep(1);
    puts("1- Go to the main menu");
    puts("2- Exit the program");

    puts("(Enter 1 or 2)");
}

void cryptoPredictor(){
    system("clear");
    sleep(1.5);
    puts("Welcome to the crypto predictor!");
    sleep(1.5);
    puts(" ");
    puts("To use, first insert the symbol of the coin");
    puts("that you want to get prediction about.");
    puts(" ");
    sleep(1.5);
    puts("After that, insert the amount you want to invest.");
    puts(" ");
    sleep(1.5);
    puts("We will show you the ROI according to our predictors.");
    puts(" ");
    puts(" ");
}

void wrongSymbolError(){
    sleep(1);
    puts("\nUnfortunately, we could not find the crypto you have mentioned");
    puts("in our database.");
    sleep(1);
    puts(" ");
    puts("Try entering another crypto.\n");
    sleep(2);
}

void predictionsLoader(){
    system("clear");
    puts("Loading...");
    sleep(2);
    system("clear");
    puts("According to the predictions, your ROI will be as follows:");
}


//gcc -o crypto3 app.c -lcurl -lsqlite3 -lssl -lcrypto