#pragma once

#include <stdio.h>
#include <stdlib.h>

#define FATAL(fmt, ...) \
	do { fprintf(stderr, "%s:%d:%s " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); exit(1); } while(0)
