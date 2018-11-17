#include "RegistryDriverLoader.h"

#include <windows.h> 
#include <winsvc.h> 
#include <conio.h> 
#include <stdio.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "../utils/Privilege.hpp"

namespace pkn
{
bool RegistryDriverLoader::enable_load_driver_privilege()
{
    return pkn::Privilege::enable_privilege(SE_LOAD_DRIVER_NAME);
}
}
