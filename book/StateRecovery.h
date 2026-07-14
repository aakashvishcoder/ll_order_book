#pragma once

#include "OrderBook.h"
#include <string>

void recoverState(OrderBook& book, const std::string& log_file_path);
