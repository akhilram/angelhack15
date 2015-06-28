#include <pebble.h>
#include "TextBlobList.h"

TextBlobNode* pebble_follow_textbloblist_push_back(TextBlobNode *tail, TextBlob *blob)
{
  TextBlobNode *newBlob = (TextBlobNode*) malloc (sizeof(TextBlobNode));
  newBlob->blob = blob;
  newBlob->next = NULL;
 
  if(tail != NULL)
    tail->next = newBlob;
  return newBlob;
}

void pebble_follow_textbloblist_erase(TextBlobNode *head)
{
  if(head != NULL)
  {
    pebble_follow_text_blob_destroy(head->blob);
    TextBlobNode *nextNode = head->next;
    free(head);
    pebble_follow_textbloblist_erase(nextNode);
  }
}
