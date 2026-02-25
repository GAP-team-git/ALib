//
//  ALib.cpp
//  ALib
//
//  Created by Giuseppe Coppini on 25/02/26.
//

#include <iostream>
#include "ALib.hpp"
#include "ALibPriv.hpp"

void ALib::HelloWorld(const char * s)
{
    ALibPriv *theObj = new ALibPriv;
    theObj->HelloWorldPriv(s);
    delete theObj;
};

void ALibPriv::HelloWorldPriv(const char * s) 
{
    std::cout << s << std::endl;
};

