#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "mfem.hpp"
#include <memory>

/**
 * Wrapper for mfem::HypreBoomerAMG solver.
 */
 class MFEMHypreDiagScale : public MFEMSolverBase
 {
 public:
  static InputParameters validParams();

  MFEMHypreDiagScale(const InputParameters &);

  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;
 
 protected:
   void constructSolver(const InputParameters & parameters) override;
 
 private:
  // const MFEMFESpace & _mfem_fespace;
  mfem::real_t _strength_threshold;
 };
 
#endif
