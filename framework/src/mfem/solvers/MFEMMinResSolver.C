#ifdef MFEM_ENABLED

#pragma once
#include "MFEMMinResSolver.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMinResSolver);

InputParameters
MFEMMinResSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEMMinResSolver solver and preconditioner for the iterative solution "
                             "of MFEM equation systems.");
  params.addParam<double>("rel_tol", 1e-12, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 2000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");
  return params;
}

MFEMMinResSolver::MFEMMinResSolver(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMMinResSolver::constructSolver(const InputParameters & /*parameters*/)
{
  auto solver = std::make_unique<mfem::MINRESSolver>( getMFEMProblem().mesh().getMFEMParMesh().GetComm() );

  solver->SetRelTol(getParam<double>("rel_tol"));
  solver->SetMaxIter(getParam<int>("l_max_its"));
  solver->SetPrintLevel(2);

  _solver = std::move(solver);
}

void
MFEMMinResSolver::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{

  if (_lor)
    mooseError("LOR solver not configured yet");

  if (_preconditioner)
  {
    _preconditioner->updateSolver(a, tdofs);
    setPreconditioner(static_cast<mfem::MINRESSolver &>(*_solver));
  }
}

#endif
