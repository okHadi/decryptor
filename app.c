#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <termios.h>

#define MAX_LEN 128
#define MAX_LINE_LENGTH 1000
#define SIZE 300

void predictionResultMenu();
void image();
void adminPanelMenu();
void adminPanel();
void adminPanelOptions();
void mainMenu();
void mainMenuOptions();
void cryptoPredictor();
void wrongSymbolError();
void predictionResult(float, float, int);
void predictionsLoader();
void aboutusMenu();
double *getPredictionPrices(char *, int , int *);
void addCryptosToTxt(char *);

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
    char *sql = "CREATE TABLE IF NOT EXIST cryptos(id text NOT NULL, year integer NOT NULL, prediction float NOT NULL);";
  
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    char finalPrice[20]; 
    /****************************INPUT FROM USER**********************************/
    menu:
    puts("");
    int errormenu = 0;
    //mainMenu();
    image();
    setting:
    puts("");
    if (errormenu == 1){
        puts("Please enter a correct option.");
    }
    errormenu = 0;
    char opt;
    char symbol[10];
    char* result;
    float priceInvested;

    scanf(" %c", &opt);

    if (opt == '1'){              //user chooses to get the prediction calculator
        cryptoPredictor();
        symbol:
        printf("Enter the id of the coin: ");


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

        if (symbolCheck == 0)
    {                  //if it does not exit, have the user enter another one/enter it again
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
                           //string where the price will be stored without any other part of the res data
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
    }



    /**************************************PREDICTION EXTRACTION****************************************************/

    int size23, size24, size25;
    double *predictions23 = getPredictionPrices(symbol, 2023, &size23);
    double *predictions24 = getPredictionPrices(symbol, 2024, &size24);
    double *predictions25 = getPredictionPrices(symbol, 2025, &size25);

    /**********************CALCULATE THE MEAN********************************/
    float avg23 = 0;
    float avg24 = 0;
    float avg25 = 0;

    for (int j=0; j< size23 ; j++){
        avg23 += predictions23[j];
    }
    for (int j=0; j< size24 ; j++){
        avg24 += predictions24[j];
    }
    for (int j=0; j< size25 ; j++){
        avg25 += predictions25[j];
    }
    avg23 = avg23/size23;
    avg24 = avg24/size24;
    avg25 = avg25/size25;

    printf("\n");
    predictionsLoader();
    printf("The current price of %s is: %s", symbol, finalPrice);
    float profit23 = (priceInvested/finPrice)*avg23;
    float profit24 = (priceInvested/finPrice)*avg24;
    float profit25 = (priceInvested/finPrice)*avg25;
    char *status;
    puts(" ");
    predictionResult(priceInvested, profit23, 2023);
    predictionResult(priceInvested, profit24, 2024);
    predictionResult(priceInvested, profit25, 2025);
    predictionResultMenu();
    predopt:
    puts("");
    char predopt;
    scanf(" %c", &predopt);
    if (predopt == '1'){
        goto menu;
    }
    else if(predopt == '2'){
        return 0;
    }
    else{
        puts("Please enter a correct option.");
        goto predopt;
    }

    }



    else if(opt == '2'){              //user chooses to read the aboutus section on menu
        system("clear");
        char opt2;
        FILE* filePointer;
        int bufferLength = 255;
        char buffer[bufferLength];

        filePointer = fopen("aboutus.txt", "r");            //prints the about us file

        while(fgets(buffer, bufferLength, filePointer)) {
        usleep(500000);
        printf("%s\n", buffer);
        }

        fclose(filePointer);

        aboutusMenu();
        correctOpt:
        scanf(" %c", &opt2);
        if (opt2=='1'){           //goes from about us section to the menu
            goto menu;
        }
        else if(opt2 == '2'){         //exits the program from the about us sectoin
            return 0;
        }
        else{
            puts("Please enter a correct option (1 or 2)");     //incase user chooses wrong option from about us
            goto correctOpt;
        }

    }
    else if(opt == '3'){
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
                sleep(1);
                puts("The password is correct, logging you in...");
                adminPanel();
                adminPanelOptions();
                adminpanel:
                puts("");
                int adminPanelOpt;
                scanf("%d", &adminPanelOpt);
                if (adminPanelOpt == 1){            //add crypto predictions
                    addprediction:
                    puts("");
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
                    addCryptosToTxt(idadd);
                    wrongoptpred:
                    puts("1- Add another prediction");
                    puts("2- See the predictions");
                    puts("3- Exit to main menu");
                    char afterpred;
                    scanf(" %c", &afterpred);
    
                    if (afterpred == '1'){
                        goto addprediction;
                    }


                    else if(afterpred == '2'){
                        totalpreds:
                        char *sql = "SELECT * FROM cryptos;";

                        sqlite3_stmt *stmt;

                        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

                        if (rc != SQLITE_OK ) {
                        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                        sqlite3_close(db);

                        return 1;
                        } 

                        // Print the column names
                        int num_columns = sqlite3_column_count(stmt);

                        for (int i = 0; i < num_columns; i++) {
                        printf("%s ", sqlite3_column_name(stmt, i));
                        }

                        printf("\n");

                        // Print the rows
                        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                            for (int i = 0; i < num_columns; i++) {
                                switch (sqlite3_column_type(stmt, i)) {
                                    case SQLITE_INTEGER:
                                    printf("%d ", sqlite3_column_int(stmt, i));
                                    break;
                                    case SQLITE_FLOAT:
                                    printf("%lf ", sqlite3_column_double(stmt, i));
                                    break;
                                    case SQLITE_TEXT:
                                    printf("%s ", sqlite3_column_text(stmt, i));
                                    break;
                                    default:
                                    printf("NULL ");
                                }
                            }

                            printf("\n");
                        }

                        if (rc != SQLITE_DONE) {
                        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                        sqlite3_close(db);
                        }
                        goto wrongoptpred;
                        sqlite3_finalize(stmt);
                    }
                    


                    else if(afterpred =='3'){
                        goto menu;
                    }


                    else{
                        puts("\nPlease enter a correct option.");
                    }

                }
                else if(adminPanelOpt == 3){
                    goto menu;
                }
                else if(adminPanelOpt ==2){
                    goto totalpreds;
                }
                else{
                    puts("Please enter a correct option.");
                    goto adminpanel;
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
    else if (opt == '4'){
        
        sqlite3_close(db);
        return 0;                                    //user chooses to exit the program from the menu
    }
    else{                                           //incase user inputs wrong option on the menu
        //puts("Please enter a correct option.\n");
        errormenu = 1;
        goto setting;
   
    }


}


void addCryptosToTxt(char *symbol){
      FILE *fp;
  
    // Open the file in append mode
    fp = fopen("predictions.txt", "a");
  
    if (fp == NULL) {
        fprintf(stderr, "Error opening file\n");
    }
    // Write the string to the file
    fprintf(fp, symbol);
  
    // Close the file
    fclose(fp);
}


void predictionResult(float priceInvested, float profityear, int year){
    if (profityear>priceInvested){
        printf("By %d: %.2f increased to %.2f USD \n", year, priceInvested, profityear);
        printf("Percetage increase: %.2f\n", ((profityear-priceInvested)/priceInvested)*100);
        printf("\n");
    }
    else{
        printf("By %d: %.2f decreased to %.2f USD \n", year, priceInvested, profityear);
        printf("Percetage decrease: %.2f\n", ((profityear-priceInvested)/priceInvested)*100);
        printf("\n");
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
    puts("Here you can add your crypto predictions, or view existing predictions.");
}

void adminPanelOptions(){
    sleep(1);
    puts("Select one of the following options:");
    puts("1- Insert predictions");
    puts("2- View existing predictions");
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

void image(){
    system("clear");
    sleep(1.5);
    char *filename = "menu.txt";
    FILE *fptr = NULL;
 
    if((fptr = fopen(filename,"r")) == NULL)
    {
        fprintf(stderr,"error opening %s\n",filename);
        return 1;
    }
 
    char read_string[MAX_LEN];

    while(fgets(read_string,sizeof(read_string),fptr) != NULL)
    printf("%s",read_string);
 
    fclose(fptr);

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

    sleep(1);
        system("clear");
    sleep(1.5);
    char *filename = "cryptopredictor.txt";
    FILE *fptr = NULL;
 
    if((fptr = fopen(filename,"r")) == NULL)
    {
        fprintf(stderr,"error opening %s\n",filename);
    }
 
    char read_string[MAX_LEN];

    while(fgets(read_string,sizeof(read_string),fptr) != NULL)
    printf("%s",read_string);
 
    fclose(fptr);

    sleep(1.5);
    sleep(1.5);
    puts(" ");
    puts("How to use:");
    puts("First insert the symbol of the coin");
    puts("that you want to get prediction about.");
    puts(" ");
    sleep(1.5);
    puts("After that, insert the amount you want to invest.");
    puts(" ");
    sleep(1.5);
    puts("We will show you the ROI according to our predictors.");

    puts("Available predictions:");
        sqlite3 *db;
    int rc;

    // Open the database
    rc = sqlite3_open("cryptos.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }

    // Execute the SELECT statement
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, "SELECT DISTINCT id FROM cryptos", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

    }

    // Print the resulting rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("id: %s\n", sqlite3_column_text(stmt, 0));
    }
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error stepping through results: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

    }
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
    printf("\n");
}


double *getPredictionPrices(char *symbol, int year, int *num_values){       //num values will store the number of elements in the returned array
    
    sqlite3 *db;
    int rc = sqlite3_open("cryptos.db", &db);
    char *sql = "SELECT prediction FROM cryptos WHERE id = ? AND year = ?;";

    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK ) {
    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);

    return NULL;
    } 

    // Bind the values to the placeholders
    sqlite3_bind_text(stmt, 1, symbol, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, year);

    // Count the number of rows in the result set
    int count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
    count++;
    }

    *num_values = count;

    if (count == 0) {
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return NULL;
    }

    // Allocate memory for the float array
    double *predictions = malloc(count * sizeof(double));

    if (predictions == NULL) {
    fprintf(stderr, "Error allocating memory for float array\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return NULL;
    }

    // Reset the prepared statement and fetch the rows again
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_bind_text(stmt, 1, symbol, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, year);

    int i = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
    // Get the float value
    double float_value = sqlite3_column_double(stmt, 0);

    predictions[i++] = float_value;
    }

    sqlite3_finalize(stmt);
    //sqlite3_close(db);

    return predictions;

}

void predictionResultMenu(){
    sleep(1);
    puts("\nWhat would you like to do next?");
    puts("1- Go to main menu");
    puts("2- Exit the program");
}



//gcc -o crypto app.c -lcurl -lsqlite3 -lssl -lcrypto