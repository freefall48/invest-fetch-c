#include "nzx.h"

const char* NZXURL = "https://www.nzx.com/markets/NZSX";


static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  memoryChunk_t *mem = (memoryChunk_t *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

memoryChunk_t*
FetchNZXData(void)
{
    CURL *curl_handle;
    CURLcode res;

    memoryChunk_t* chunk = malloc(sizeof (memoryChunk_t));

    chunk->memory = malloc(1);
    chunk->size = 0; 

    curl_handle = curl_easy_init();
    // Set the options for this curl handler
    curl_easy_setopt(curl_handle, CURLOPT_URL, NZXURL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.132 Safari/537.36");

    res = curl_easy_perform(curl_handle);
    // Check for errors
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    } else {
        printf("%lu bytes retrieved\n", (unsigned long)chunk->size);
    }

    // Clean up
    curl_easy_cleanup(curl_handle);
    // free(chunk->memory);
    // free(chunk);
    return chunk;

}

void
ProcessListing(char* ptr)
{

}

void 
ExtractListings(memoryChunk_t* chunk)
{
  char* ptr;

  ptr = strstr(chunk->memory, "<tbody>");
  FILE *f;
  f = fopen("test.txt", "w");

  fprintf(f, "%s", ptr);
  fclose(f);
}

void
freeMemoryChunk(memoryChunk_t* chunk)
{
  free(chunk->memory);
  free(chunk);
}