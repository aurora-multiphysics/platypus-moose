#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorPostprocessor.h"

/*
 * MFEM Postprocessor which samples values at points.
 */
class MFEMPointValueSampler : public MFEMVectorPostprocessor
{
public:
  static InputParameters validParams();

  MFEMPointValueSampler(const InputParameters & parameters);

  virtual void finalize() override {}
};

#endif // MFEM_ENABLED
