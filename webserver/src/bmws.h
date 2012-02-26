#include "common.h"
#ifdef _WIN32
    #include <winsock2.h>
#endif
#ifndef BMWS_H
#define BMWS_H

#define SMALL_BUFSIZE 256
#define BUFSIZE 4096
#define SUBST_BUFSIZE 20480

#define DEFAULT_PORT 2605
#define MIN_PORT 1
#define MAX_PORT 65535
#define INVALID_SOCKET -1

#define MIME_JSON "application/json"
#define MIME_HTML "text/html"
#define MIME_TEXT "text/plain"
#define MIME_CSV  "text/csv"
#define MIME_JPEG "image/jpeg"
#define MIME_GIF  "image/gif"
#define MIME_PNG  "image/png"
#define MIME_ICO  "image/vnd.microsoft.icon"
#define MIME_JS   "application/x-javascript"
#define MIME_CSS  "text/css"
#define MIME_BIN  "application/octet-stream"
#define MIME_XML  "application/xhtml+xml"

#define SYNC_CONTENT_TYPE   "application/vnd.codebox.bitmeter-sync"
#define HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_EOL "\r\n"

struct HttpResponse{
    int   code;
    char* msg;
};

struct Request{
    char* method;
    char* path;
    struct NameValuePair* params;
    struct NameValuePair* headers;
};
struct Request* parseRequest(char* requestTxt);
void freeRequest(struct Request* request);

struct MimeType{
    char* fileExt;
    char* contentType;
    int binary;
};

struct WebConnectionConfig {
    int port;
    int allowRemoteConnect;
    int allowRemoteAdmin;
};

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
void processMobileMonitorRequest(SOCKET fd, struct Request* req);
void processMobileMonitorAjaxRequest(SOCKET fd, struct Request* req);
void processSummaryRequest(SOCKET fd, struct Request* req);
void processMobileSummaryRequest(SOCKET fd, struct Request* req);
void processQueryRequest(SOCKET fd, struct Request* req);
void processSyncRequest(SOCKET fd, struct Request* req);
void processConfigRequest(SOCKET fd, struct Request* req, int allowAdmin);
void processExportRequest(SOCKET fd, struct Request* req);
void processAlertRequest(SOCKET fd, struct Request* req, int allowAdmin);
void processFileRequest(SOCKET fd, struct Request* req, struct NameValuePair* substPairs);
void processRssRequest(SOCKET fd, struct Request* req);
struct NameValuePair* makeRssRequestValues();

void writeText(SOCKET fd, char* txt);
void writeData(SOCKET fd, char* data, int len);
void writeDataToJson(SOCKET fd, struct Data* data);
void writeSingleDataToJson(SOCKET fd, struct Data* data);
void writeTextValueToJson(SOCKET fd, char* key, char* value);
void writeTextArrayToJson(SOCKET fd, char* key, char** values);
void writeNumValueToJson(SOCKET fd, char* key, BW_INT value);
void writeSyncData(SOCKET fd, struct Data* data);
void writeFilterData(SOCKET fd, struct Filter* filter);
void writeHeadersOk(SOCKET fd, char* contentType, int endHeaders);
void writeHeadersSeeOther(SOCKET fd, struct Request* req, int endHeaders);
void writeHeadersNotFound(SOCKET fd, char* file);
void writeHeadersForbidden(SOCKET fd, char* request);
void writeHeadersNotAllowed(SOCKET fd, char* httpMethod);
void writeHeadersServerError(SOCKET fd, char* msg, ...);
void writeHeader(SOCKET fd, char* name, char* value);
void writeEndOfHeaders(SOCKET fd);
void processRequest(SOCKET fd, char* buffer, int allowAdmin);

void getWebRoot(char* path);
int getPort();
int isLocalConnection(SOCKET socket);
struct WebConnectionConfig readDbConfig();

#ifdef UNIT_TESTING
    #define WRITE_HEADERS_SERVER_ERROR mockWriteHeadersServerError
    #define WRITE_HEADERS_FORBIDDEN mockWriteHeadersForbidden
    #define WRITE_HEADERS_OK mockWriteHeadersOk
    #define WRITE_TEXT mockWriteText
    #define WRITE_NUM_VALUE_TO_JSON mockWriteNumValueToJson
    #define WRITE_TEXT_VALUE_TO_JSON mockWriteTextValueToJson
    #define WRITE_TEXT_ARRAY_TO_JSON mockWriteTextArrayToJson
    #define WRITE_HEADER mockWriteHeader
    #define WRITE_END_OF_HEADERS mockWriteEndOfHeaders
    #define WRITE_DATA_TO_JSON mockWriteDataToJson
    #define FREAD mockFread
    #define WRITE_DATA mockWriteData
    #define WRITE_FILTER_DATA mockWriteFilterData
    #define WRITE_SYNC_DATA mockWriteSyncData
#else
    #define WRITE_HEADERS_SERVER_ERROR writeHeadersServerError
    #define WRITE_HEADERS_FORBIDDEN writeHeadersForbidden
    #define WRITE_HEADERS_OK writeHeadersOk
    #define WRITE_TEXT writeText
    #define WRITE_NUM_VALUE_TO_JSON writeNumValueToJson
    #define WRITE_TEXT_VALUE_TO_JSON writeTextValueToJson
    #define WRITE_TEXT_ARRAY_TO_JSON writeTextArrayToJson
    #define WRITE_HEADER writeHeader
    #define WRITE_END_OF_HEADERS writeEndOfHeaders
    #define WRITE_DATA_TO_JSON writeDataToJson
    #define FREAD fread
    #define WRITE_DATA writeData
    #define WRITE_FILTER_DATA writeFilterData
    #define WRITE_SYNC_DATA writeSyncData
#endif

#endif // ifndef BMWS_H
