#pragma once
#include "TextBlob.h"
  
typedef struct TextBlobNodeStruct {
  TextBlob *blob;
  struct TextBlobNodeStruct *next;
} TextBlobNode;


TextBlobNode* pebble_follow_textbloblist_push_back(TextBlobNode *tail, TextBlob *blob);
void pebble_follow_textbloblist_erase(TextBlobNode *head);
