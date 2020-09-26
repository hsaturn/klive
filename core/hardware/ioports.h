#pragma once

#include <common/observer.h>

class Ioports : public Observable<Ioports>
{
public:
    Ioports();
};

