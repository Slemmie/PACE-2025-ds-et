#pragma once

#include <string>

std::string file_read(std::string_view filepath);

void file_write(std::string_view filepath, std::string_view data);
