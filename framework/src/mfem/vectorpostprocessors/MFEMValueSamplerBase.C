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

mfem::Vector
points_to_mfem_vector(const std::vector<Point> & points, mfem::Ordering::Type ordering)
{
  const unsigned int num_points = points.size();
  const unsigned int num_dims = LIBMESH_DIM;
  mfem::Vector mfem_points(num_points * num_dims);
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++)
    {
      size_t idx;
      if (ordering == mfem::Ordering::byNODES)
      {
        idx = i_dim * num_points + i_point;
      }
      else // ordering == mfem::Ordering::byVDIM
      {
        idx = i_point * num_dims + i_dim;
      }

      mfem_points(idx) = points[i_point](i_dim);
    }
  }

  return mfem_points;
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
    _interp_vals(points.size() * this->getMFEMProblem().mesh().getMFEMParMesh().SpaceDimension()),
    _var_name(getParam<VariableName>("variable")),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(_var_name)),
    _declared_value(this->declareVector(_var_name))
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
    _declared_points.push_back(this->declareVector("x_" + std::to_string(i)));
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
  // TODO: copy interpolated points to declared vectors
}

MFEMValueSamplerBase::~MFEMValueSamplerBase() { _finder.FreeData(); }

#endif // MOOSE_MFEM_ENABLED
