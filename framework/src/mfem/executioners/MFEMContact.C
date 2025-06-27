#ifdef MFEM_ENABLED
#include "MFEMContact.h"
#include "MFEMProblem.h"

#include "axom/slic.hpp"
#include "tribol/interface/tribol.hpp"
#include "tribol/interface/mfem_tribol.hpp"

/*
  We should be able to inherit all of this
*/

registerMooseObject("MooseApp", MFEMContact);

InputParameters
MFEMContact::validParams()
{
  InputParameters params = MFEMExecutioner::validParams();
  params.addClassDescription("Executioner for steady state MFEM problems.");
  params.addParam<Real>("time", 0.0, "System time");
  return params;
}

MFEMContact::MFEMContact(const InputParameters & params)
  : MFEMExecutioner(params),
    _system_time(getParam<Real>("time")),
    _time_step(_mfem_problem.timeStep()),
    _time(_mfem_problem.time()),
    _output_iteration_number(0)
{
  std::cout << "Initialising the Executioner\n";
  _time = _system_time;
}

void
MFEMContact::constructProblemOperator()
{
  _problem_data.eqn_system = std::make_shared<Moose::MFEM::EquationSystem>();
  auto problem_operator    = std::make_unique<Moose::MFEM::EquationSystemProblemOperator>(_problem_data);
  
  _problem_operator.reset();
  _problem_operator = std::move(problem_operator);
}

void
MFEMContact::init()
{
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();

  initTribol();

  // Set up initial conditions
  // makes and registers some pargrid functions
  _problem_data.eqn_system->Init(
    _problem_data.gridfunctions,
    _problem_data.fespaces,
    getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>()
  );

  _problem_operator->SetGridFunctions();
  _problem_operator->Init(_problem_data.f);
}

void
MFEMContact::initTribol()
{
  // silence warning
  axom::slic::initialize();

  // Create a Tribol coupling scheme: defines contact surfaces and enforcement
  int coupling_scheme_id = 0;
  
  const int dimensions = _mfem_problem.mesh().dimension();
  tribol::initialize(dimensions, MPI_COMM_WORLD);

  // While there is a single mfem ParMesh for this problem, Tribol
  // defines a mortar and a nonmortar contact mesh, each with a unique mesh ID.
  // The Tribol mesh IDs for each contact surface are defined here.
  int mesh1_id = 0;
  int mesh2_id = 1;

  // take a reference to the pmesh
  mfem::ParMesh& pmesh = *(_mfem_problem.getProblemData().pmesh);

  tribol::registerMfemCouplingScheme(
    coupling_scheme_id, mesh1_id, mesh2_id,
    pmesh, _mfem_problem.getCoords(), mortar_attrs, nonmortar_attrs,
    tribol::SURFACE_TO_SURFACE,
    tribol::NO_CASE,
    tribol::SINGLE_MORTAR,
    tribol::FRICTIONLESS,
    tribol::LAGRANGE_MULTIPLIER,
    tribol::BINNING_GRID
  );

  // Set Tribol options for Lagrange multiplier enforcement
  tribol::setLagrangeMultiplierOptions(
    coupling_scheme_id,
    tribol::ImplicitEvalMode::MORTAR_RESIDUAL_JACOBIAN
  );
 
  // #4: Update contact mesh decomposition so the on-rank Tribol meshes
  // coincide with the current configuration of the mesh. This must be called
  // before tribol::update().
  tribol::updateMfemParallelDecomposition();
  
  // #5: Update contact gaps, forces, and tangent stiffness contributions
  int cycle = 1;   // pseudo cycle
  mfem::real_t t = 1.0;  // pseudo time
  mfem::real_t dt = 1.0; // pseudo dt
  tribol::update(cycle, t, dt);
}

void
MFEMContact::execute()
{
  _time_step = 0;
  _time = _time_step;
  _mfem_problem.outputStep(EXEC_INITIAL);
  _time = _system_time;

  preExecute();

  _mfem_problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;
  _mfem_problem.timestepSetup();

  _problem_operator->Solve(_problem_data.f);
  
  auto& displacement_true = _problem_operator->_true_x.GetBlock(0);
  // get fespace
  auto& fespace      = _problem_data.fespaces.GetRef("H1FESpace");
  auto& displacement = _problem_data.gridfunctions.GetRef("displacement");

  fespace.GetProlongationMatrix()->Mult(displacement_true,displacement);
  displacement.Neg();

  _mfem_problem.getProblemData().pmesh->EnsureNodes();
  mfem::ParGridFunction& coords = *dynamic_cast<mfem::ParGridFunction*>( _mfem_problem.getProblemData().pmesh->GetNodes() );
  coords += displacement;

  // all this is shutdown stuff
  tribol::updateMfemParallelDecomposition();
  tribol::finalize();

  // Displace mesh, if required
  _mfem_problem.displaceMesh();

  _mfem_problem.computeIndicators();
  _mfem_problem.computeMarkers();

  // need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);
  _mfem_problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _mfem_problem.execMultiApps(EXEC_FINAL);
    _mfem_problem.finalizeMultiApps();
    _mfem_problem.postExecute();
    _mfem_problem.execute(EXEC_FINAL);
    _time = _time_step;
    _mfem_problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}

#endif
