#ifdef MFEM_ENABLED

#pragma once
#include "MFEMHypreDiagScale.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreDiagScale);

InputParameters
MFEMHypreDiagScale::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Hypre DiagScale solver and preconditioner for the iterative solution "
                             "of MFEM equation systems.");
  params.addParam<double>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>(
      "fespace", "H1 FESpace to ");
  return params;
}

MFEMHypreDiagScale::MFEMHypreDiagScale(const InputParameters & parameters)
  : MFEMSolverBase(parameters)//, _mfem_fespace(getUserObject<MFEMFESpace>("fespace"))
{
  mfem::Hypre::Init();
  constructSolver(parameters);
}

void
MFEMHypreDiagScale::constructSolver(const InputParameters & /*parameters*/)
{
  // this is supposed to be constructed with a hyprepar matrix
  auto solver = std::make_unique<mfem::HypreDiagScale>();

  // solver->SetTol(getParam<double>("l_tol"));
  // solver->SetMaxIter(getParam<int>("l_max_its"));
  // solver->SetPrintLevel(getParam<int>("print_level"));
  // solver->SetStrengthThresh(_strength_threshold);

  // if (preconditioner)
  //   setPreconditioner(*solver);

  _solver = std::move(solver);
}

void
MFEMHypreDiagScale::updateSolver(mfem::ParBilinearForm & /*a*/, mfem::Array<int> & /*tdofs*/)
{
  if (_lor)
    mooseError("LOR solver not configured yet");

}

#endif
