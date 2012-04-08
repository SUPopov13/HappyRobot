#ifndef __METAPROGRAMMING_CONCATINATION__
#define __METAPROGRAMMING_CONCATINATION__

#include <cstring>
#include <stdint.h>

#include "sequence.h"

namespace robot { namespace metaprogramming_tools
{    
    namespace concatination
    {
        template <typename ...Args>
        struct _cat;

        template <typename ...Args0, typename ...Args1>
        struct _cat<sequence<Args0...>, sequence<Args1...>>
        {
            using type = sequence<Args0..., Args1...>;
        };
    }

    template <typename S0, typename S1>
    using concatinate = typename concatination::_cat<S0, S1>::type;
}}

#endif // __METAPROGRAMMING_CONCATINATION__
