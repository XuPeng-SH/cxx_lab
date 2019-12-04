#pragma once
#include <stdio.h>
#include <iostream>

#define LOGD(MSG) do {                  \
    std::cout << "DEBUG | " << MSG << " (" << __FILE__  << ":" << __LINE__ << ")" << std::endl; \
} while(0)
