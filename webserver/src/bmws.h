/*
 * BitMeterOS v0.3.2
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build Date: Sun, 07 Mar 2010 14:49:47 +0000
 */

#include "common.h"
#ifdef _WIN32
	#include <winsock2.h>
#endif
#ifndef BMWS_H
#define BMWS_H

#define BUFSIZE 4096
#define HEADER_BUFSIZE 256

#define MIME_JSON "application/json"
#define MIME_HTML "text/html"
#define MIME_JPEG "image/jpeg"
#define MIME_GIF  "image/gif"
#define MIME_PNG  "image/png"
#define MIME_ICO  "image/vnd.microsoft.icon"
#define MIME_JS   "application/x-javascript"
#define MIME_CSS  "text/css"
#define MIME_BIN  "application/octet-stream"

#define SYNC_CONTENT_TYPE   "application/vnd.codebox.bitmeter-sync"
#define HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_EOL "\r\n"

struct HttpResponse{
    int   code;
    char* msg;
};

struct NameValuePair{
    char* name;
    char* value;
    struct NameValuePair* next;
};
long getValueNumForName(char* name, struct NameValuePair* pair, long defaultValue);
char* getValueForName(char* name, struct NameValuePair* pair, char* defaultValue);

struct Request{
	char* method;
    char* path;
    struct NameValuePair* params;
    struct NameValuePair* headers;
};
struct Request* parseRequest(char* requestTxt);
void freeRequest(struct Request* request);

#ifdef _WIN32
	void initMutex();
	void waitForMutex();
	void releaseMutex();
	void setupWeb();
	void shutdownWeb();
	void processWeb();
	#define WEB_SERVICE_NAME "BitMeterWebService"
#endif

void processMonitorRequest(SOCKET fd, struct Request* req);
void processSummaryRequest(SOCKET fd, struct Request* req);
void processQueryRequest(SOCKET fd, struct Request* req);
void processSyncRequest(SOCKET fd, struct Request* req);
void processConfigRequest(SOCKET fd, struct Request* req);
void processFileRequest(SOCKET fd, struct Request* req);

void writeText(SOCKET fd, char* txt);
void writeDataToJson(SOCKET fd, struct Data* data);
void writeSingleDataToJson(SOCKET fd, struct Data* data);
void writeSyncData(SOCKET fd, struct Data* data);
void writeHeaders(SOCKET fd, struct HttpResponse response, char* contentType, int size);
void processRequest(SOCKET fd, char* buffer);


#endif
