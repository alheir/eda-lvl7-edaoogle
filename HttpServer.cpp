/**
 * @file HttpServer.h
 * @author Marc S. Ressl
 * @brief Simple interface to libmicrohttpd
 * @version 0.1
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "HttpServer.h"

using namespace std;

/**
 * @brief GetArgument callback for libmicrohttp
 *
 * @param cls The return variable
 * @param kind Source of key-value pairs
 * @param key The key
 * @param value The value
 * @return int #MHD_YES to continue iterating,
 *             #MHD_NO to abort the iteration
 */
static MHD_Result httpGetArgumentCallback(void *cls,
                                          enum MHD_ValueKind kind,
                                          const char *key,
                                          const char *value)
{
    HttpArguments *arguments = (HttpArguments *)cls;

    if (value != NULL)
        (*arguments)[key] = value;
    else
        (*arguments)[key] = "";

    return MHD_YES;
}

/**
 * @brief HTTP request handler for libmicrohttpd
 *
 * @param cls The server object
 * @param connection The connection
 * @param url The URL
 * @param method The HTTP method
 * @param version The HTTP version
 * @param upload_data Data uploaded
 * @param upload_data_size side of data uploaded
 * @param con_cls Pointer that the callback can set
 * @return MHD_Result
 */
MHD_Result httpRequestHandlerCallback(void *cls,
                                      struct MHD_Connection *connection,
                                      const char *url,
                                      const char *method,
                                      const char *version,
                                      const char *upload_data,
                                      size_t *upload_data_size,
                                      void **con_cls)
{
    HttpServer *server = (HttpServer *)cls;

    // Headers are invalid on first call, wait for second call.
    if (con_cls == NULL)
    {
        *con_cls = cls;

        return MHD_YES;
    }

    // We only handle get requests
    if ((string(method) == "GET"))
    {
        // Get arguments
        HttpArguments arguments;
        MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, httpGetArgumentCallback, &arguments);

        // Make response
        int statusCode;
        vector<char> response;

        // Clean URL
        string cleanedUrl = url;
        if (cleanedUrl == "")
            cleanedUrl = "/";

        // Convert directories to files
        if (cleanedUrl.back() == '/')
            cleanedUrl += "index.html";

        if (server->httpRequestHandler &&
            server->httpRequestHandler->handleRequest(cleanedUrl, arguments, response))
            statusCode = MHD_HTTP_FOUND;
        else
        {
            statusCode = MHD_HTTP_NOT_FOUND;

            string errorResponse = "<html><body><h1>404 Not Found</h1></body></html>";
            response.assign(errorResponse.begin(), errorResponse.end());
        }

        MHD_Response *mhdResponse = MHD_create_response_from_buffer(response.size(),
                                                                    (void *)response.data(),
                                                                    MHD_RESPMEM_MUST_COPY);
        bool isResponseQueued = MHD_queue_response(connection, statusCode, mhdResponse);
        MHD_destroy_response(mhdResponse);

        return isResponseQueued ? MHD_YES : MHD_NO;
    }

    return MHD_NO;
}

HttpServer::HttpServer(int port)
{
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD,
                              8000,
                              NULL,
                              NULL,
                              httpRequestHandlerCallback,
                              this,
                              MHD_OPTION_END);

    httpRequestHandler = NULL;
}

HttpServer::~HttpServer()
{
    if (daemon)
        MHD_stop_daemon(daemon);

    httpRequestHandler = NULL;
}

bool HttpServer::isRunning()
{
    return daemon != NULL;
}

void HttpServer::setHttpRequestHandler(HttpRequestHandler *httpRequestHandler)
{
    this->httpRequestHandler = httpRequestHandler;
}
