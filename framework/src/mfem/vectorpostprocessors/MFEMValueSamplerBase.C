//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once

#include "MFEMValueSamplerBase.h"
#include "MFEMProblem.h"

InputParameters
MFEMValueSamplerBase::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();

  params.addRequiredParam<VariableName>(
      "variable", "The names of the variables that this VectorPostprocessor operates on");

  return params;
}

MFEMValueSamplerBase::MFEMValueSamplerBase(const InputParameters & parameters,
                                           mfem::Vector && points, size_t num_points)
  : MFEMVectorPostprocessor(parameters),
    _finder(this->comm().get()),
    _points(std::move(points)),
    _interp_vals(points.Size()),
    _var_name(getParam<VariableName>("variable")),
    _var(getMFEMProblem().getProblemData().gridfunctions.GetRef(_var_name))
{
  auto & mesh = this->getMFEMProblem().mesh().getMFEMParMesh();
  _finder.Setup(mesh);
  const auto ordering =
      this->getMFEMProblem().mesh().getMFEMParMesh().GetNodalFESpace()->GetOrdering();
  _finder.FindPoints(_points, ordering);
  // TODO: declareVector some points and value vectors
}

void MFEMValueSamplerBase::execute() {
  _finder.Interpolate(_var, _interp_vals);
}

void MFEMValueSamplerBase::finalize() {
   if (_interp_vals.UseDevice())
   {
      _interp_vals.HostReadWrite();
   }
   _points.HostReadWrite();
   //TODO: copy interpolated points to declared vectors
}

MFEMValueSamplerBase::~MFEMValueSamplerBase() { _finder.FreeData(); }

#endif // MFEM_ENABLED
