/*
 * @Descripttion: the API to start server.
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: kkcoding@qq.com
 * @Date: 2019-06-26 15:46:20
 * @LastEditors: Kevin
 * @LastEditTime: 2019-06-26 18:52:27
 */

#ifndef _SERVER_API_H_
#define _SERVER_API_H_

#include <esp_http_server.h>

/**
 * @brief: starter of server, it will register all file services to server host.
 * @param  NULL
 * @return: the handler of server.
 */
httpd_handle_t start_server_core(void);
void stop_server_core(httpd_handle_t server);

#endif /* _SERVER_API_H_ */
