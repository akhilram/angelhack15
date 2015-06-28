#include <pebble.h>

typedef struct
{
  char *text_blob;
  int length;
  int current_pointer;
} TextBlob;

void pebble_follow_text_blob_create(const char *string, TextBlob **blob);
const char* pebble_follow_text_blob_get_next_word(TextBlob *blob);
void pebble_follow_text_blob_destroy(TextBlob *blob);
void pebble_follow_text_blob_reset(TextBlob *blob);

