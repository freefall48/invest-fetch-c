/*
 * Copyright (c) 1996 - 2020, Daniel Stenberg, daniel@haxx.se, and many contributors, see the THANKS file.
 * All rights reserved. Permission to use, copy, modify, and distribute this software for any purpose with or
 * without fee is hereby granted, provided that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall not be used in advertising or otherwise
 * to promote the sale, use or other dealings in this Software without prior written authorization of the
 * copyright holder.
 *
 * Modified 29/03/2020 - Matthew Johnson
 * */

#include "httpOps.h"

static size_t
writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t i = size * nmemb;
    memoryChunk_t *mem = (memoryChunk_t *) userp;

    char *ptr = realloc(mem->memory, mem->size + i + 1);
    if (ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, i);
    mem->size += i;
    mem->memory[mem->size] = 0;

    return i;
}

memoryChunk_t *
nzxFetchData(void) {
    CURL *curl_handle;
    CURLcode res;

    memoryChunk_t *chunk = malloc(sizeof(memoryChunk_t));

    chunk->memory = malloc(1);
    chunk->size = 0;

    curl_handle = curl_easy_init();
    // Set the options for this curl handler
    curl_easy_setopt(curl_handle, CURLOPT_URL, NZX_URL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *) chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT,
                     "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.132 Safari/537.36");

    res = curl_easy_perform(curl_handle);
    // Check for errors
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }
//    else {
//        printf("%lu bytes retrieved\n", (unsigned long) chunk->size);
//    }

    // Clean up and return
    curl_easy_cleanup(curl_handle);
    return chunk;

}

void
nzxFreeMemoryChunk(memoryChunk_t *chunk) {
    free(chunk->memory);
    free(chunk);
}
