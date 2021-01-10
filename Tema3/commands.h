

void register_command(int sockfd, char* command);
char* get_cookie(char* response);
char* login_command(int sockfd, char* command);
void add_book_command(int sockfd, char* token, char* command);
bool check_number(char* string);
char* acces_command(char* cookie, int sockfd, char* command);
void get_books_command(int sockfd, char* token, char* command);
void get_book_command(int sockfd, char* token, char* command);
void delete_book_command(int sockfd, char* token, char* command);
void logout_command(int sockfd, char* cookie, char* command);