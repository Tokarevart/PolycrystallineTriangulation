#pragma once
#include "monon.h"

namespace spt {

template <std::size_t Dim = 3, typename Real = typename spt::vec<Dim>::real_type>
using vertex = monon<Dim, Real>;

} // namespace spt
