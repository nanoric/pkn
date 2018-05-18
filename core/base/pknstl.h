#pragma once

#ifdef USE_STDSTL
namespace pknstl = pknstl;
#else /*use EASTL*/
namespace eastl
{
}
namespace pknstl
{
    using namespace eastl;
}
#endif
