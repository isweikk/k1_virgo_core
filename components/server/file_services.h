/*
 * @Descripttion: services for access file system.
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: kkcoding@qq.com
 * @Date: 2019-06-26 15:46:20
 * @LastEditors: Kevin
 * @LastEditTime: 2019-06-26 15:59:22
 */

#ifndef _FILE_SERVICES_H_
#define _FILE_SERVICES_H_

#include <esp_http_server.h>

/**
 * @brief: register all file services to server host.
 * @param {in} server: the handler you created
 *        {in} base_path: the root of the file system
 * @return: 
 *      - ESP_OK
 *      - ESP_FAIL
 */
esp_err_t file_services_register(httpd_handle_t server, const char *base_path);

#endif /* _FILE_SERVICES_H_ */
