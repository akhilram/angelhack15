#include "TextBlob.h"

void pebble_follow_text_blob_create(const char *string, TextBlob **blob)
{
  *blob = (TextBlob*)malloc(sizeof(TextBlob));
  (*blob)->length = strlen(string);
  app_log(1,"TextBlob", 6, "String: %s", string);
  
  (*blob)->text_blob = (char *)malloc((*blob)->length);
  memset((*blob)->text_blob, '\0', (*blob)->length);
  strcpy((*blob)->text_blob, string);
  
  (*blob)->current_pointer = 0;
}

bool is_delimiter(char c)
{
  switch(c) {
    case ' ':
    case '.':
    case ',':
      return true;
    default:
      return false;
  }
  return false;
}

const char* pebble_follow_text_blob_get_next_word(TextBlob *blob)
{
  char word[100]; //100 max word length
  if (!blob->text_blob)
    return "";
  
  //Skip all delimiters
  while(blob->current_pointer < blob->length && is_delimiter(*(blob->text_blob + blob->current_pointer))) 
  {
    blob->current_pointer++;
  }
  
  int i=0;
  while(blob->current_pointer < blob->length && i<100 && !is_delimiter(*(blob->text_blob + blob->current_pointer)))
  {
    word[i++] = *(blob->text_blob + blob->current_pointer);
    blob->current_pointer++;
  }
  
   blob->current_pointer++;
  
  word[i++] = '\0';
  char *string="";
  strcpy(string, word);
  
  return string;
}


void pebble_follow_text_blob_destroy(TextBlob *blob)
{
  free(blob->text_blob);
  free(blob);  
}


