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
extractMarketListings(memoryChunk_t* chunk, NZXNode_t** head)
{
  char* ptr;

  // Move the pointer to the start of the datatable
  ptr = strstr(chunk->memory, "<tbody>");
  // Check there is actually a table in the HTML downloaded
  if (!ptr) {
    return;
  }

  const char* CODE_STRING = "<tr class=\"\" title=\"";
  const int CODE_SIZE = 3;

  const char* COMPANY_STRING = " <a title=\"";

  // Move the ptr to the first data row
  ptr = strstr(ptr, CODE_STRING);

  while (ptr)
  {
    listing_t listing;

    // Extract the listing code
    ptr += strlen(CODE_STRING);
    listing.Code = malloc((CODE_SIZE + 1) * sizeof(char));
    listing.Code[CODE_SIZE] = '\0';
    strncpy(listing.Code, ptr, 3);

    // printf("%s\n", listing.Code);

    // Find the end of the company name
    ptr = strstr(ptr, COMPANY_STRING);
    ptr += strlen(COMPANY_STRING);
    char* end = strchr(ptr + 1, '"');
    int companyLength = (int)(end - ptr);

    listing.Company = malloc((companyLength + 1) * sizeof(char));
    listing.Company[companyLength] = '\0';
    strncpy(listing.Company, ptr, companyLength);

    // printf("%s\n", listing.Company);


    pushListing(head, listing);

    // Move the ptr to the next data row if it exists
    ptr = strstr(ptr, CODE_STRING);

  }
}

void
freeMemoryChunk(memoryChunk_t* chunk)
{
  free(chunk->memory);
  free(chunk);
}