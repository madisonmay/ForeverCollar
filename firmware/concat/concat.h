void send_string(char* s);
char* concat(char*, char*);

void send_string(char* s) {
  int length = strlen(s);
  for (int i=0; i<length; i++) {
    send_byte(s[i]);
  }
  break_and_flush();
}

char* concat(char *s1, char *s2)
{
    //string concatenation -- not needed now, but might prove useful later
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    //should also check for memory allocation errors here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
