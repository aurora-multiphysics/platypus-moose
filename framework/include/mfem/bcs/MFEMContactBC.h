#ifdef MFEM_ENABLED

#pragma once
#include "MFEMEssentialBC.h"

class MFEMContactBC : public MFEMEssentialBC
{
public:
  static InputParameters validParams();

  MFEMContactBC(const InputParameters & parameters);
  ~MFEMContactBC() override = default;

  void ApplyBC(mfem::GridFunction & gridfunc) override;

};

#endif
