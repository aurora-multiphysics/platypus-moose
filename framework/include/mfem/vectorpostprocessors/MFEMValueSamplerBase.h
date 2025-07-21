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

#include "MooseTypes.h"
#include "mfem.hpp"

/*
 * MFEM Postprocessor which samples values at points.
 *
 * Subclasses should override validParams and provide and use the constructor.
 */
class MFEMValueSamplerBase : public MFEMVectorPostprocessor
{
public:
  static InputParameters validParams();

  MFEMValueSamplerBase(const InputParameters & parameters, mfem::Vector && points, size_t num_points);
  // mfem::FindPointsGSLIB's default copy constructor does not correctly handle
  // the fdataD data member so make this class uncopyable
  MFEMValueSamplerBase(const MFEMValueSamplerBase & other) = delete;
  MFEMValueSamplerBase & operator=(const MFEMValueSamplerBase & other) = delete;
  virtual ~MFEMValueSamplerBase();

  /** Perform the interpolation in FindPointsGSLIB.
   */
  virtual void execute() override;

  /** Store the result of the interpolation.
   */
  virtual void finalize() override;

private:
  mfem::FindPointsGSLIB _finder;
  mfem::Vector _points;
  mfem::Vector _interp_vals;

  const VariableName & _var_name;
  const mfem::GridFunction & _var;
};

#endif // MFEM_ENABLED
