#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { FALSE = 0, TRUE = 1 } BOOL;

const char *types[]      = {"void", "int", "char", "float"};
const char *qualifiers[] = {"const", "extern", "volatile"};
const char *structures[] = {"union", "struct", "enum"};

void cdcl();

BOOL find_name();
void read_name();

BOOL is_type();
BOOL is_qualifier();
BOOL is_structure();

BOOL check_postfix();
BOOL read_array();

BOOL check_prefix();
BOOL read_paren();

BOOL check_qualifier();
BOOL check_left_qualifier();
BOOL check_type();

struct {
    int length;
    int front_offset;
    int back_offset;
    char str[200];
} dcl = { 0, 0, 0, "\0" };

enum Token_Type { ERR, PAREN, BRACKET, POINTER, QUALIFIER, TYPE };
struct {
    enum Token_Type token_type;
    BOOL SUCCESS;
    BOOL ERROR;
    char name[50];
} token = { ERR, FALSE, FALSE, "\0" };

int main(int argc, char *argv[])
{
    if (argc == 1) {
        while (fgets(dcl.str, 200, stdin)) {
            dcl.length = strlen(dcl.str);
            dcl.front_offset = 0;
            token.token_type = ERR;
            token.SUCCESS = FALSE;
            token.ERROR = FALSE;

            cdcl();
        }
    } else if (argc == 2) {
        strcpy(dcl.str, argv[1]);
        dcl.length = strlen(dcl.str);
        cdcl();
    } else {
        printf ("Error, too many arguments!");
        return 1;
    }
    
    return 0;
}

void cdcl()
{
    char prev_qualifier[50] = "\0";
    char type_name[50] = "\0";
    
    if (find_name()) {
        printf("%s is ", token.name);
      
        BOOL CHECK_POSTFIX = TRUE;
        BOOL CHECK_PREFIX = TRUE;

        while(CHECK_POSTFIX) {
            CHECK_POSTFIX = FALSE;
            CHECK_PREFIX = TRUE;
            if (check_postfix()) {
                if (token.token_type == PAREN) {
                    printf("function taking %s returning ", token.name);
                }
                else if (token.token_type == BRACKET) {
                    while (read_array()) {
                        printf("array%s of ", token.name);
                    }
                    if (!token.ERROR) {
                        printf("array%s of ", token.name);
                    }
                }
            }
            if (token.ERROR) {
                CHECK_PREFIX = FALSE;
            }
            while(CHECK_PREFIX) {
                CHECK_PREFIX = FALSE;
                if (check_prefix()) {
                    if (token.token_type == PAREN) {
                        if (read_paren()) {
                            CHECK_POSTFIX = TRUE;
                        }
                    }
                    else if (token.token_type == POINTER) {
                        printf("pointer to ");
                        CHECK_PREFIX = TRUE;
                    }
                    else if (token.token_type == QUALIFIER) {
                        strcpy(prev_qualifier, token.name);
                        printf("%s ", token.name);
                        CHECK_PREFIX = TRUE;
                    }
                    else if (token.token_type == TYPE) {
                        strcpy(type_name, token.name);
                        if (check_left_qualifier()) {
                            if (!strcmp(prev_qualifier, token.name)) {
                                token.SUCCESS = FALSE;
                            } else {
                                token.SUCCESS = TRUE;
                                printf("%s %s", token.name, type_name);
                            }
                        } else if (!token.ERROR) {
                            token.SUCCESS = TRUE;
                            printf("%s", type_name);
                        }
                    }
                }
            }
        }
        printf("\n");
    }
    else {
        printf("Error: Name not found\n");
    }
    if (token.ERROR || !token.SUCCESS) {
        if (token.token_type == PAREN) {
            printf("Error: Missing parentheses\n");
        } else if (token.token_type == BRACKET) {
            printf("Error: Missing square bracket\n");
        } else if (token.token_type == QUALIFIER ||
                   token.token_type == TYPE) {
            printf("Error: Unknown type or qualifier\n");
        } else {
            printf("Error: Unknown error\n");
        }
    }
    
}

BOOL find_name()
{
    int i = dcl.front_offset;
    while (i <= dcl.length && !isalpha(dcl.str[i])) {
        i++;
    }
    dcl.front_offset = i;
    dcl.back_offset = dcl.front_offset;
    read_name();

    if (!is_type() && !is_qualifier() && !is_structure()) {
        return TRUE;    // found name
    } else {
        return find_name();
    }
    return FALSE;
}

void read_name()
{
    int i = dcl.front_offset + 1;
    while (isalnum(dcl.str[i]) && i <= dcl.length) {
        i++;
    }
    for (int j = dcl.front_offset; j < i; j++) {
        token.name[j-dcl.front_offset] = dcl.str[j];
    }
    token.name[i-dcl.front_offset] = '\0';

    dcl.front_offset = i;
}

BOOL is_type()
{
    for (unsigned int i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
        if (!strcmp(token.name, types[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL is_qualifier()
{
    for (unsigned int i = 0; i < sizeof(qualifiers)/sizeof(qualifiers[0]); i++) {
        if (!strcmp(token.name, qualifiers[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL is_structure()
{
    for (unsigned int i = 0; i < sizeof(structures)/sizeof(structures[0]); i++) {
        if (!strcmp(token.name, structures[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL check_postfix()
{
    int i = dcl.front_offset;
    
    while (i <= dcl.length && isspace(dcl.str[i])) {
        i++;
    }
    dcl.front_offset = i;
    if (dcl.str[i] == '(') {
        token.token_type = PAREN;
        token.name[0] = '(';
            
        for (int j = 1; j > 0; ) {
            i++;
            if (dcl.str[i] == ')') {
                j--;
            } else if (dcl.str[i] == '(') {
                j++;
            }
            token.name[i-dcl.front_offset] = dcl.str[i];
            if (i > dcl.length) {
                token.ERROR = TRUE;
                return FALSE;
            }
        }
        token.name[i-dcl.front_offset+1] = '\0';
        dcl.front_offset = i + 1;
        return TRUE;
    }
    else if (dcl.str[i] == '[') {
        token.token_type = BRACKET;
        dcl.front_offset = i;
        return TRUE;
    }
    else {
        return FALSE;
    }
}

BOOL read_array()
{
    int i = dcl.front_offset;
    
    token.name[0] = '[';
    while (dcl.str[i] != ']') {
        token.name[i-dcl.front_offset] = dcl.str[i];
        ++i;
        if (i > dcl.length) {
            token.ERROR = TRUE;
            return FALSE;
        }
    }
    token.name[i-dcl.front_offset] = ']';
    token.name[i-dcl.front_offset+1] = '\0';
    
    while (i <= dcl.length) {
        if (dcl.str[i] == '[') {
            dcl.front_offset = i;
            return TRUE;
        }
        i++;
    }
    return FALSE;
}

BOOL check_prefix()
{
    int i = dcl.back_offset - 1;
    
    while (i > 0 && isspace(dcl.str[i])) {
        i--;
    }
    if (dcl.str[i] == '(') {
        token.token_type = PAREN;
        dcl.back_offset = i;
        if (read_paren()) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    else if (dcl.str[i] == '*') {
        token.token_type = POINTER;
        dcl.back_offset = i;
        return TRUE;
    }
    else if (isalpha(dcl.str[i])) {
        dcl.back_offset = i;
        while (i > 0 && isalpha(dcl.str[i-1])) {
            i--;
        }
        for(int j = 0; j <= dcl.back_offset - i; j++) {
            token.name[j] = dcl.str[i + j];
        }
        token.name[dcl.back_offset-i+1] = '\0';
        dcl.back_offset = i;
        
        if (is_type()) {
            token.token_type = TYPE;
            return TRUE;
        }
        else if (is_qualifier()) {
            token.token_type = QUALIFIER;
            return TRUE;
        }
        else {
            token.token_type = TYPE;
            token.ERROR = TRUE;
            return FALSE;
        }
    }
    else {
        return FALSE;
    }
}

BOOL read_paren()
{
    int i = dcl.back_offset + 1;
    
    for (int j = 1; j > 0; ) {
        i++;
        if (dcl.str[i] == ')') {
            j--;
        } else if (dcl.str[i] == '(') {
            j++;
        }
        if (i > dcl.length) {
            token.ERROR = TRUE;
            return FALSE;
        }
    }
    dcl.front_offset = i + 1;
    return TRUE;
}

BOOL check_left_qualifier()
{
    int i = dcl.back_offset - 1;
    
    while (i > 0 && isspace(dcl.str[i])) {
        i--;
    }
    if (isalpha(dcl.str[i])) {
        dcl.back_offset = i;
        while (i > 0 && isalpha(dcl.str[i-1])) {
            i--;
        }
        for(int j = 0; j <= dcl.back_offset - i; j++) {
            token.name[j] = dcl.str[i + j];
        }
        token.name[dcl.back_offset-i+1] = '\0';
        dcl.back_offset = i;
        
        if (is_qualifier()) {
            return TRUE;
        } else {
            token.ERROR = TRUE;
        }
    }
    return FALSE;
}

