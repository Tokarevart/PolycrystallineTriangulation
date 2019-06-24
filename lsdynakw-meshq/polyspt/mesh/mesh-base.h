#pragma once
#include "../simplex.h"
#include <vector>
#include <memory>

namespace spt {

template <typename ElemType>
struct mesh_base;

template <typename ElemType>
using unique_mesh = mesh_base<std::unique_ptr<ElemType>>;

template <typename ElemType>
using raw_mesh = mesh_base<ElemType*>;

template <typename MeshType>
using mesh_composition_base = std::vector<MeshType>;

template <typename ElemType>
using unique_mesh_composition = mesh_composition_base<std::unique_ptr<unique_mesh<ElemType>>>;

template <typename ElemType>
using raw_mesh_composition = mesh_composition_base<raw_mesh<ElemType>*>;

} // namespace pmg
