/*
 * Copyright (c) 2014 CNRS/LAAS
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include <mpower/mpower.h>
#include "cJSON.h"

struct MemoryStruct {
	char *memory;
	size_t size;
};

struct mPowerDev {
	char *host;
	struct MemoryStruct *chunk;
	CURL *handle;
	struct mPowerStatus *outputs;
};

/*
 * Callback function for libcurl's request:
 * store HTTP answers in a memory buffer
 */
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return -1;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

#ifdef MPOWER_DEBUG
/*
 * Dump HTTP answers to screen
 */
static void
PrintMemory(struct MemoryStruct *chunk)
{
	printf("chunk.size: %ld\n", (long)chunk->size);
	printf("chunk: %s\n", chunk->memory);
}
#endif

/*
 * (Re)set memory buffer to a single '\0'
 */
static void
ResetMemory(struct MemoryStruct *chunk)
{
	free(chunk->memory);
	chunk->memory = malloc(1);
	chunk->size = 0;
}

/*
 * Parse the json data returned by GetOutputs
 */
static int
mPowerParseStatus(struct mPowerDev *dev)
{
	char *data = dev->chunk->memory;
	char *status;
	cJSON *json, *sensors, *sensor, *value;
	int i, n;
	struct mPowerStatus *outputs = dev->outputs;

	json = cJSON_Parse(data);
	if (json == NULL) {
		fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	status = cJSON_GetObjectItem(json, "status")->valuestring;
	if (strcmp(status, "success") != 0) {
		fprintf(stderr, "Error retreiving the outputs status: %s\n", 
		    status);
		return -1;
	}
	
	sensors = cJSON_GetObjectItem(json, "sensors");
	n = cJSON_GetArraySize(sensors);
	for (i = 0; i < n; i++) {
		sensor = cJSON_GetArrayItem(sensors, i);
		outputs[i].port = cJSON_GetObjectItem(sensor, "port")->valueint;		value = cJSON_GetObjectItem(sensor, "id");
		if (value!= NULL && value->valuestring != NULL) {
			if (strcmp(outputs[i].id, value->valuestring) != 0) {
				free(outputs[i].id);
				outputs[i].id = strdup(value->valuestring);
			}
		} else {
			free(outputs[i].id);
			outputs[i].id = NULL;
		}
		value = cJSON_GetObjectItem(sensor, "label");
		if (value != NULL && value->valuestring != NULL)  {
			if (strcmp(outputs[i].label,
				value->valuestring) != 0) {
				free(outputs[i].label);
				outputs[i].label = strdup(value->valuestring);
			}
		} else {
			free(outputs[i].label);
			outputs[i].label = NULL;
		}
		outputs[i].power = cJSON_GetObjectItem(sensor,
		    "power")->valuedouble;
		outputs[i].energy = cJSON_GetObjectItem(sensor,
		    "energy")->valuedouble;
		outputs[i].current = cJSON_GetObjectItem(sensor,
		    "current")->valuedouble;
		outputs[i].voltage = cJSON_GetObjectItem(sensor,
		    "voltage")->valuedouble;
		outputs[i].powerfactor = cJSON_GetObjectItem(sensor,
		    "powerfactor")->valuedouble;
		outputs[i].relay = cJSON_GetObjectItem(sensor,
		    "relay")->valueint;
	}
	cJSON_Delete(json);
	return 0;
}

/*
 * Initialize a connection to the mPower web server
 */
struct mPowerDev *
mPowerInit(const char *host)
{
	struct mPowerDev *dev;

	dev = (struct mPowerDev *)malloc(sizeof(struct mPowerDev));
	if (dev == NULL)
		return NULL;
	dev->chunk = (struct MemoryStruct *)malloc(sizeof(struct MemoryStruct));
	if (dev->chunk == NULL) {
		free(dev);
		return NULL;
	}
	memset(dev->chunk, 0, sizeof(struct MemoryStruct));
	dev->host = strdup(host);
	if (dev->host == NULL) {
		free(dev->chunk);
		free(dev);
		return NULL;
	}
	dev->outputs = calloc(MPOWER_NUM_OUTPUTS, sizeof(struct mPowerStatus));
	if (dev->outputs == NULL) {
		free(dev->host);
		free(dev->chunk);
		free(dev);
		return NULL;
	}
	curl_global_init(CURL_GLOBAL_ALL);
	dev->handle = curl_easy_init();

	// Set up a couple initial parameters
	// that we will not need to mofiy later.
	curl_easy_setopt(dev->handle, CURLOPT_USERAGENT, "Mozilla/4.0");
	curl_easy_setopt(dev->handle, CURLOPT_AUTOREFERER, 1 );
	curl_easy_setopt(dev->handle, CURLOPT_FOLLOWLOCATION, 1 );
	curl_easy_setopt(dev->handle, CURLOPT_COOKIEFILE, "");

	/* send all data to this function  */
	curl_easy_setopt(dev->handle, CURLOPT_WRITEFUNCTION,
	    WriteMemoryCallback);
 	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(dev->handle, CURLOPT_WRITEDATA, dev->chunk);

	return dev;
}

/*
 * terminate the connexion to the mpower web server
 */
void
mPowerEnd(struct mPowerDev *dev)
{
	int i;

	curl_easy_cleanup(dev->handle);
	free(dev->chunk->memory);
	free(dev->chunk);
	free(dev->host);
	for (i = 0; i < MPOWER_NUM_OUTPUTS; i++) {
		free(dev->outputs[i].id);
		free(dev->outputs[i].label);
	}
	free(dev->outputs);
	free(dev);
}

/*
 * perform login to the web server
 */
int
mPowerLogin(struct mPowerDev *dev,
    const char *user, const char *password)
{
	char data[40], url[1024];
	long code;

	// Visit the login page
	ResetMemory(dev->chunk);
	snprintf(url, sizeof(url), "http://%s/index.cgi", dev->host);
	curl_easy_setopt(dev->handle, CURLOPT_URL, url);
	curl_easy_perform(dev->handle);
	curl_easy_getinfo(dev->handle, CURLINFO_RESPONSE_CODE, &code);
	if (code != 200) {
		fprintf(stderr, "error connecting to login page\n");
		return 1;
	}

	// Next we tell LibCurl what HTTP POST data to submit
	snprintf(data, sizeof(data), "username=%s&password=%s", user, password);
	curl_easy_setopt(dev->handle, CURLOPT_POSTFIELDS, data);
	snprintf(url, sizeof(url), "http://%s/login.cgi", dev->host);
	curl_easy_setopt(dev->handle, CURLOPT_URL, url);
	curl_easy_perform(dev->handle);
	curl_easy_getinfo(dev->handle, CURLINFO_RESPONSE_CODE, &code);
	if (code != 200 ||
	    strstr(dev->chunk->memory, "Invalid credentials.") != NULL) {
		printf("login error\n");
		return -1;
	}
	return 0;
}

int
mPowerSetOutput(struct mPowerDev *dev, int output, int value)
{
	char enable[20];
	char url[1024];
	long code;
	cJSON *json;
	char *status;

	ResetMemory(dev->chunk);
	snprintf(enable, sizeof(enable), "output=%d", value);
	curl_easy_setopt(dev->handle, CURLOPT_POSTFIELDS, enable);
	curl_easy_setopt(dev->handle, CURLOPT_CUSTOMREQUEST, "PUT");
	snprintf(url, sizeof(url), "http://%s/sensors/%d/", dev->host, output);
	curl_easy_setopt(dev->handle, CURLOPT_URL, url);
	curl_easy_perform(dev->handle);
	curl_easy_getinfo(dev->handle, CURLINFO_RESPONSE_CODE, &code);
#ifdef MPOWER_DEBUG
	PrintMemory(dev->chunk);
#endif
	if (code != 200 || dev->chunk->memory[0] != '{') {
		printf("code: %ld\n", code);
		return -1;
	}
	json = cJSON_Parse(dev->chunk->memory);
	if (json == NULL) {
		fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	status = cJSON_GetObjectItem(json, "status")->valuestring;
	if (strcmp(status, "success") != 0) {
		fprintf(stderr, "Error retreiving the outputs status: %s\n", 
		    status);
		return -1;
	}
	cJSON_Delete(json);
	return 0;
}

int
mPowerQueryOutputs(struct mPowerDev *dev)
{
	char url[1024];
	long code;

	ResetMemory(dev->chunk);
	curl_easy_setopt(dev->handle, CURLOPT_CUSTOMREQUEST, NULL);
	curl_easy_setopt(dev->handle, CURLOPT_HTTPGET, 1);
	snprintf(url, sizeof(url), "http://%s/mfi/sensors.cgi", dev->host);
	curl_easy_setopt(dev->handle, CURLOPT_URL, url);
	curl_easy_perform(dev->handle);
	curl_easy_getinfo(dev->handle, CURLINFO_RESPONSE_CODE, &code);
#ifdef MPOWER_DEBUG
	PrintMemory(dev->chunk);
#endif
	if (code != 200 || dev->chunk->memory[0] != '{') {
		printf("code: %ld\n", code);
		return -1;
	}
	return mPowerParseStatus(dev);
}

void
mPowerPrintOutputs(struct mPowerDev *dev, FILE *out)
{
	struct mPowerStatus *status = dev->outputs;
	int i;

	for (i = 0; i < MPOWER_NUM_OUTPUTS; i++) {
		printf("%d: %s %s %.4lf W %.4lf A %.4lf V %.4lf %d\n",
		    status[i].port,
		    status[i].id != NULL ? status[i].id : "''",
		    status[i].label != NULL ? status[i].label : "''",
		    status[i].power, status[i].current,
		    status[i].voltage, status[i].powerfactor,
		    status[i].relay);
	}
}

void
mPowerGetOutputs(struct mPowerDev *dev, struct mPowerStatus *outputs)
{
	struct mPowerStatus *output;
	int i;

	memcpy(outputs, dev->outputs,
	    MPOWER_NUM_OUTPUTS*sizeof(struct mPowerStatus));
	/* copy strings if needed */
	for (i = 0; i < MPOWER_NUM_OUTPUTS; i++) {
		output = &dev->outputs[i];
		if (output->id != NULL)
			outputs[i].id = strdup(output->id);
		if (output->label != NULL)
			outputs[i].label = strdup(output->label);
	}
}
