#include <string>

namespace CLI
{
    std::string GetSlamhausLogo()
    {
        return R"(
   ▄████████  ▄█          ▄████████   ▄▄▄▄███▄▄▄▄      ▄█    █▄       ▄████████ ███    █▄     ▄████████ 
  ███    ███ ███         ███    ███ ▄██▀▀▀███▀▀▀██▄   ███    ███     ███    ███ ███    ███   ███    ███ 
  ███    █▀  ███         ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███   ███    █▀  
  ███        ███         ███    ███ ███   ███   ███  ▄███▄▄▄▄███▄▄   ███    ███ ███    ███   ███        
▀███████████ ███       ▀███████████ ███   ███   ███ ▀▀███▀▀▀▀███▀  ▀███████████ ███    ███ ▀███████████ 
         ███ ███         ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███          ███ 
   ▄█    ███ ███▌    ▄   ███    ███ ███   ███   ███   ███    ███     ███    ███ ███    ███    ▄█    ███ 
 ▄████████▀  █████▄▄██   ███    █▀   ▀█   ███   █▀    ███    █▀      ███    █▀  ████████▀   ▄████████▀  
             ▀                                                                                          
        )";
    }

    std::string GetInfo()
    {
        return R"(
    Creator: Syed Assad Abbas Bokhari
    Website: https://www.piledriver-playhouse.com/slamhaus
    Version: 0.1.0
    
    License: MIT License
    Copyright (c) 2025 Piledriver Playhouse Limited
    )";
    }
    
}
