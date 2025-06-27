#ifdef MFEM_ENABLED

#include "MFEMContactBC.h"

registerMooseObject("MooseApp", MFEMContactBC);

InputParameters
MFEMContactBC::validParams()
{
  return MFEMEssentialBC::validParams();
}

MFEMContactBC::MFEMContactBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters)
{
}

void
MFEMContactBC::ApplyBC(mfem::GridFunction & /*gridfunc*/)
{

}

#endif
