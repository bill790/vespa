// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include "emptyreply.h"
#include "errorcode.h"
#include "ireplyhandler.h"

namespace mbus {

Routable::Routable() :
    _context(),
    _stack(),
    _trace()
{
    // empty
}

void
Routable::discard()
{
    _context = Context();
    _stack.discard();
    _trace.clear();
}

void
Routable::swapState(Routable &rhs)
{
    std::swap(_context, rhs._context);
    _stack.swap(rhs._stack);
    _trace.swap(rhs._trace);
}

} // namespace mbus
