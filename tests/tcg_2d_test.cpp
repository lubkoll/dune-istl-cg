#include <gtest/gtest.h>

#include <cmath>

#include <dune/istl/scalarproducts.hh>

#include "mock/linearOperator_2d.hh"
#include "mock/trivialPreconditioner.hh"
#include "mock/vector.hh"

#include "../tcg_solver.hh"
#include "../residual_based_termination_criterion.hh"

/*
 * Test conjugate gradient method with the example given at:
 *
 *   https://en.wikipedia.org/wiki/Conjugate_gradient_method#Numerical_example
 *
 */

namespace Mock = Dune::Mock;
using Mock::Vector;

namespace
{
  struct ScalarProduct : Dune::ScalarProduct<Vector>
  {
    static constexpr int category = Dune::SolverCategory::sequential;

    typename Dune::ScalarProduct<Vector>::field_type dot(const Vector& x, const Vector& y) final override
    {
      double result = 0;
      for ( std::size_t i = 0; i < x.data_.size(); ++i )
        result += x.data_[i] * y.data_[i];
      return result;
    }

   double norm(const Vector& x) final override
    {
      return sqrt(dot(x,x));
    }
  };

  struct TestTCGSolver_2d : ::testing::Test
  {
    TestTCGSolver_2d()
      : A(), P(), sp(),
        cg( A, P, sp )
    {
    }

    Dune::Mock::LinearOperator_2d A;
    Dune::Mock::TrivialPreconditioner P;
    ScalarProduct sp;
    Dune::TCGSolver< Vector, Vector , Dune::KrylovTerminationCriterion::ResidualBased > cg;
  };

  Vector initialGuess()
  {
    return Vector( { 2., 1. } );
  }

  Vector rightHandSide()
  {
    return Vector( { 1., 2. } );
  }
}

TEST_F(TestTCGSolver_2d,NoStep)
{
  cg.setMaxSteps(0);
  auto x = initialGuess();
  auto b = rightHandSide();

  cg.apply(x,b);

  // initial residual
  ASSERT_DOUBLE_EQ( b.data_[0], -8 );
  ASSERT_DOUBLE_EQ( b.data_[1], -3 );
}

TEST_F(TestTCGSolver_2d,OneStep)
{
  cg.setMaxSteps(1);
  auto x = initialGuess();
  auto b = rightHandSide();

  cg.apply(x,b);

  double alpha = 73.0/331;

  // residual
  ASSERT_DOUBLE_EQ( b.data_[0], -8 + alpha * 35 );
  ASSERT_DOUBLE_EQ( b.data_[1], -3 + alpha * 17 );

  // first iterate
  ASSERT_DOUBLE_EQ( x.data_[0], 2 + alpha * -8 );
  ASSERT_DOUBLE_EQ( x.data_[1], 1 + alpha * -3 );
}


TEST_F(TestTCGSolver_2d,TwoSteps)
{
  cg.setMaxSteps(2);
  auto x = initialGuess();
  auto b = rightHandSide();

  cg.apply(x,b);

  // second iterate
  ASSERT_DOUBLE_EQ( x.data_[0], 0.0909090909090909 );
  ASSERT_DOUBLE_EQ( x.data_[1], 0.6363636363636364 );
}
