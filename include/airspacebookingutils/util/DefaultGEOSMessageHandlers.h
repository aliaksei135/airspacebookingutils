/*
 * DefaultGEOSMessageHandlers.h
 *
 *  Created by A.Pilko on 23/04/2021.
 */

#ifndef UASGROUNDRISK_SRC_UTILS_DEFAULTGEOSMESSAGEHANDLERS_H_
#define UASGROUNDRISK_SRC_UTILS_DEFAULTGEOSMESSAGEHANDLERS_H_

#include <cstdarg>
#include <stdio.h>
#include <cstdlib>
#include <spdlog/spdlog.h>

static void notice(const char* fmt, ...)
{
	spdlog::info("GEOS Notice: " + std::string(fmt));
}

static void log_and_exit(const char* fmt, ...)
{
	spdlog::critical("GEOS Error: " + std::string(fmt));
}

#endif // UASGROUNDRISK_SRC_UTILS_DEFAULTGEOSMESSAGEHANDLERS_H_
