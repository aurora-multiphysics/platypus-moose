//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once

#include "MFEMLineValueSampler.h"

#include "libmesh/point.h"
#include "MooseError.h"
#include "MFEMProblem.h"

mfem::Vector
generateLinePoints(const Point & start_point,
                   const Point & end_point,
                   unsigned int num_points,
                   unsigned int num_dims,
                   mfem::Ordering::Type ordering)
{
  if (LIBMESH_DIM != num_dims)
  {
    mooseError("In MFEMLineValueSampler: point dimension not equal to mesh dimension.");
  }

  if (num_points < 2) {
    mooseError("In MFEMLineValueSampler: line must have at least 2 points.");
  }

  // initialize and populate vector with linearly-spaced points along line
  mfem::Vector points(num_points * num_dims);
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++) {
      size_t idx;
      if (ordering == mfem::Ordering::byNODES)
      {
        idx = i_dim * num_points + i_point;
      }
      else // ordering == mfem::Ordering::byVDIM
      {
        idx = i_point * num_dims + i_dim;
      }

      // fractional distance along line [0, 1]
      Real t = static_cast<Real>(i_point) / static_cast<Real>(num_points - 1);
      points(idx) = t*end_point(i_dim) + (1 - t) * start_point(i_dim);
    }
  }

  return points;
}

InputParameters
MFEMLineValueSampler::validParams()
{
  InputParameters params = MFEMValueSamplerBase::validParams();

  params.addClassDescription("Sample an MFEM variable along a specified line.");

  // these should not be of type libmesh::Point - need mfem::Point parsing
  params.addRequiredParam<Point>("start_point", "The beginning of the line");
  params.addRequiredParam<Point>("end_point", "The ending of the line");

  params.addRequiredParam<unsigned int>("num_points",
                                        "The number of points to sample along the line");

  return params;
}

MFEMLineValueSampler::MFEMLineValueSampler(const InputParameters & parameters)
  : MFEMValueSamplerBase(
        parameters,
        generateLinePoints(
            getParam<Point>("start_point"),
            getParam<Point>("end_point"),
            getParam<unsigned int>("num_points"),
            this->getMFEMProblem().mesh().dimension(),
            this->getMFEMProblem().mesh().getMFEMParMesh().GetNodalFESpace()->GetOrdering()),
        getParam<unsigned int>("num_points"))
{
}

#endif // MFEM_ENABLED
