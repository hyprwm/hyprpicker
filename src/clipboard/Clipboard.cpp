#include "Clipboard.hpp"
#include "../includes.hpp"

void Clipboard::copy(std::string str) {

    std::stringstream copy_cmd;
    copy_cmd << "/usr/bin/wl-copy " << str << std::endl;
    std::system(copy_cmd.str().c_str());
}