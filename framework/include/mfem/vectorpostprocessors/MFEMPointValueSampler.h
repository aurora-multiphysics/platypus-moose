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

#include "MFEMVectorPostprocessor.h"

#include "mfem.hpp"

/*
 * MFEM Postprocessor which samples values at points.
 */
class MFEMPointValueSampler : public MFEMVectorPostprocessor
{
public:
  static InputParameters validParams();

  MFEMPointValueSampler(const InputParameters & parameters);
  ~MFEMPointValueSampler();

  virtual void finalize() override {}

private:
  mfem::FindPointsGSLIB finder;
};

#endif // MFEM_ENABLED
