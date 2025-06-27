//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "EquationSystemProblemOperator.h"

namespace Moose::MFEM
{

/*
  Overwriting this. If we are using the contact stuff (which for now we can assume)
  then we need to make sure the extra blocks/offsets are shoe-horned into true_x and
  true_rhs.

  The EquationSystem right now holds all the important stuff, and the EquationSystem
  isn't in scope in the parent class. So we paste in the important stuff here with
  some additions
*/
void
EquationSystemProblemOperator::SetGridFunctions()
{
  _trial_var_names = GetEquationSystem()->TrialVarNames();

  if ( GetEquationSystem()->UseContact() )
  {
    // stuff from ProblemOperatorInterface::SetGridFunctions()
    _test_variables = _problem.gridfunctions.Get(_test_var_names);
    _trial_variables = _problem.gridfunctions.Get(_trial_var_names);

    // Set operator size and block structure - add an extra one for the contact block
    _block_true_offsets.SetSize(_trial_variables.size() + 2);
    _block_true_offsets[0] = 0;
    for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
    {
      _block_true_offsets[ind + 1] = _trial_variables.at(ind)->ParFESpace()->TrueVSize();
    }

    // final one. This makes sure the contact block is registered with _true_x and _true_rhs
    _block_true_offsets[ _trial_variables.size() + 1 ] = GetEquationSystem()->getMfemPressure().ParFESpace()->TrueVSize();

    _block_true_offsets.PartialSum();

    _true_x.Update(_block_true_offsets);
    _true_rhs.Update(_block_true_offsets);
    
    // final step from ProblemOperator::SetGridFunctions()
    width = height = _block_true_offsets[_trial_variables.size()];
  }
  else // this is the normal behaviour
    ProblemOperator::SetGridFunctions();
}

void
EquationSystemProblemOperator::Init(mfem::BlockVector & X)
{
  ProblemOperator::Init(X);

  GetEquationSystem()->BuildEquationSystem();
}

void
EquationSystemProblemOperator::Solve(mfem::Vector &)
{
  GetEquationSystem()->BuildJacobian(_true_x, _true_rhs);

  if (_problem.jacobian_solver->isLOR() && _equation_system->_test_var_names.size() > 1)
    mooseError("LOR solve is only supported for single-variable systems");

  _problem.jacobian_solver->updateSolver(
      *_equation_system->_blfs.Get(_equation_system->_test_var_names.at(0)),
      _equation_system->_ess_tdof_lists.at(0));

  _problem.nonlinear_solver->SetSolver(_problem.jacobian_solver->getSolver());
  _problem.nonlinear_solver->SetOperator(*GetEquationSystem());

  // #pragma message "Warning - hardcoding _true_x block 1 to 0.0 here"
  // _true_x.GetBlock(1) = 0.0;

  _problem.nonlinear_solver->Mult(_true_rhs, _true_x);

  GetEquationSystem()->RecoverFEMSolution(_true_x, _problem.gridfunctions);
}

} // namespace Moose::MFEM

#endif
