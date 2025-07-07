#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "VectorPostprocessor.h"

/*
 * Vector postprocessor for MFEM results. Must inherit from VectorPostprocessor
 * in order for MOOSE to call it.
 */
class MFEMVectorPostprocessor : public MFEMGeneralUserObject, public VectorPostprocessor
{
public:
  static InputParameters validParams();

  MFEMVectorPostprocessor(const InputParameters & parameters);

  virtual void finalize() override {}
};

#endif // MFEM_ENABLED
