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

// paste in something from the unit test!
void
MFEMContactBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh_)
{

}

#endif
