#ifndef __SONAR_SUBSYSTEM__
#define __SONAR_SUBSYSTEM__

#include "parameter.h"

namespace robot { namespace subsystem {

    struct x_pos;
    struct y_pos;
    struct z_pos;

    struct x_vect_pos;
    struct y_vect_pos;
    struct z_vect_pos;

    struct cycle;

    struct min;
    struct max;
    struct step;

    struct value;

    template <typename Size, typename Time>
    using sonar =
    subsystem
    <
        pair<x_pos, parameter<Size>>,
        pair<y_pos, parameter<Size>>,
        pair<z_pos, parameter<Size>>,

        pair<x_vect_pos, parameter<Size>>,
        pair<y_vect_pos, parameter<Size>>,
        pair<z_vect_pos, parameter<Size>>,

        pair<cycle, parameter<Time>>,

        pair<min , parameter<Size>>,
        pair<max , parameter<Size>>,
        pair<step, parameter<Size>>,

        pair<value, parameter<Size>>
    >;
}}

#endif
