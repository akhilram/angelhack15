#pragma once
#include <pebble.h>
#include "TextBlob.h"
#include "TextBlobList.h"

void pebble_follow_add_text_blob(const char* blobText);
static void reset_blobs();
static void set_category(int category_index);
