//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "mfem/fem/fespace.hpp"
#ifdef MOOSE_MFEM_ENABLED

#include "MFEMValueSamplerBase.h"
#include "MFEMProblem.h"

namespace
{
size_t
mfem_index(const size_t i_dim,
           const size_t i_point,
           const size_t num_dims,
           const size_t num_points,
           const mfem::Ordering::Type ordering)
{
  if (ordering == mfem::Ordering::byNODES)
  {
    return i_dim * num_points + i_point;
  }
  else // ordering == mfem::Ordering::byVDIM
  {
    return i_point * num_dims + i_dim;
  }
}

mfem::Vector
points_to_mfem_vector(const std::vector<Point> & points, const mfem::Ordering::Type ordering)
{
  const unsigned int num_points = points.size();
  const unsigned int num_dims = LIBMESH_DIM;
  mfem::Vector mfem_points(num_points * num_dims);
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++)
    {
      const size_t idx = mfem_index(i_dim, i_point, num_dims, num_points, ordering);

      mfem_points(idx) = points[i_point](i_dim);
    }
  }

  return mfem_points;
}

void
mfem_vector_to_postprocessor_points(
    const mfem::Vector & mfem_points,
    std::vector<std::reference_wrapper<VectorPostprocessorValue>> & points,
    const mfem::Ordering::Type ordering)
{
  const unsigned int num_dims = LIBMESH_DIM;
  const unsigned int num_points = mfem_points.Size() / num_dims;
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++)
    {
      const size_t idx = mfem_index(i_dim, i_point, num_dims, num_points, ordering);

      points[i_dim].get()[i_point] = mfem_points(idx);
    }
  }
}
}

InputParameters
MFEMValueSamplerBase::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();

  params.addRequiredParam<VariableName>(
      "variable", "The names of the variables that this VectorPostprocessor operates on");
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>(
      "point_ordering", ordering, "Ordering style to use for point vector DoFs.");

  return params;
}

MFEMValueSamplerBase::MFEMValueSamplerBase(const InputParameters & parameters,
                                           const std::vector<Point> & points)
  : MFEMVectorPostprocessor(parameters),
    _finder(this->comm().get()),
    _points_ordering(getParam<MooseEnum>("point_ordering") == "NODES" ? mfem::Ordering::byNODES
                                                                      : mfem::Ordering::byVDIM),
    _points(points_to_mfem_vector(points, _points_ordering)),
    _interp_vals(points.size()),
    _var_name(getParam<VariableName>("variable")),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(_var_name)),
    _declared_vals(this->declareVector(_var_name))
{
  // set up points vector
  auto & mesh = this->getMFEMProblem().mesh().getMFEMParMesh();
  mesh.EnsureNodes();
  _finder.Setup(mesh);
  _finder.FindPoints(_points, _points_ordering);

  // declare points vectors for outputting
  const auto dim = this->getMFEMProblem().mesh().getMFEMParMesh().SpaceDimension();
  for (int i = 0; i < dim; i++)
  {
    std::reference_wrapper<VectorPostprocessorValue> declared_dim =
        this->declareVector("x_" + std::to_string(i));
    declared_dim.get().resize(points.size());
    _declared_points.push_back(declared_dim);
  }
}

void
MFEMValueSamplerBase::execute()
{
  _finder.Interpolate(_var, _interp_vals);
}

void
MFEMValueSamplerBase::finalize()
{
  if (_interp_vals.UseDevice())
  {
    _interp_vals.HostReadWrite();
  }
  _points.HostReadWrite();

  mfem_vector_to_postprocessor_points(_points, _declared_points, _points_ordering);
  _declared_vals.assign(_interp_vals.begin(), _interp_vals.end());
}

MFEMValueSamplerBase::~MFEMValueSamplerBase() { _finder.FreeData(); }

#endif // MOOSE_MFEM_ENABLED
