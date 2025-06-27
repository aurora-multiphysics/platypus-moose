#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "mfem.hpp"
#include <memory>

/**
 * Wrapper for mfem::HypreBoomerAMG solver.
 */
 class MFEMMinResSolver : public MFEMSolverBase
 {
public:
  static InputParameters validParams();

  MFEMMinResSolver(const InputParameters &);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

protected:
  void constructSolver(const InputParameters & parameters) override;
 
private:
  // std::shared_ptr<mfem::ParFiniteElementSpace> _mfem_fespace{nullptr};
  mfem::real_t _strength_threshold;
  std::shared_ptr<MFEMSolverBase>  _preconditioner{nullptr};
};

#endif
