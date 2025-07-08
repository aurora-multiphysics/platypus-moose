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

#include "MFEMPointValueSampler.h"

InputParameters
MFEMPointValueSampler::validParams()
{
  InputParameters params = MFEMVectorPostprocessor::validParams();

  params.addRequiredCoupledVar(
      "variable", "The names of the variables that this VectorPostprocessor operates on");
  params.addParam<PostprocessorName>(
      "scaling", 1.0, "The postprocessor that the variables are multiplied with");
  params.addParam<bool>(
      "warn_discontinuous_face_values",
      true,
      "Whether to return a warning if a discontinuous variable is sampled on a face");

  return params;
}

#endif // MFEM_ENABLED
