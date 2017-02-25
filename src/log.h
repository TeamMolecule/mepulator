#pragma once

#include <stdio.h>

#define FATAL(fmt, ...) \
	do { fprintf(stderr, "%s:%d:%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } while(0)
