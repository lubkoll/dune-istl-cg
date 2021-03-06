#ifndef DUNE_CG_HH
#define DUNE_CG_HH

#include <cassert>
#include <memory>
#include <utility>

#include "generic_iterative_method.hh"
#include "generic_step.hh"
#include "relative_energy_termination_criterion.hh"
#include "mixins/iterativeRefinements.hh"

namespace Dune
{
  namespace CGSpec
  {
    //! Cache object for the conjugate gradient method.
    template <class Domain, class Range>
    struct Cache
    {
      using real_type = real_t<Domain>;

      Cache( Domain& x0, Range& b0 )
        : x(x0), r(b0),
          Pr(x0), dx(x0),
          Adx(b0)
      {}

      void reset(LinearOperator<Domain,Range>* A_,
                Preconditioner<Domain,Range>* P_,
                ScalarProduct<Domain>* sp_)
      {
        A = A_;
        P = P_;
        sp = sp_;
        A->applyscaleadd(-1,x,r);
        P->apply(Pr,r);
        residualNorm = sp->norm ( r );
        firstStep = true;
      }

      Domain& x;
      Range& r;
      real_type alpha = -1, beta = -1, sigma = -1, dxAdx = -1, residualNorm = 1;
      Domain Pr, dx;
      Range Adx;
      bool firstStep = true;

      LinearOperator<Domain,Range>* A = nullptr;
      Preconditioner<Domain,Range>* P = nullptr;
      ScalarProduct<Domain>* sp = nullptr;
    };


    //! @cond
    class Name
    {
    public:
      std::string name() const
      {
        return "Conjugate Gradients";
      }
    };
    //! @endcond


    /**
     * @brief Extends internal interface of GenericStep for the conjugate gradient method.
     *
     * @warning Interface is only valid as long as a suitable cache object is provided.
     * Caches are typically destroyed when the computation of an iterative method is finished.
     */
    template <class Cache_, class NameOfAlgorithm = Name>
    class InterfaceImpl : public NameOfAlgorithm
    {
    public:
      using Cache = Cache_;

      void setCache(Cache* cache)
      {
        cache_ = cache;
      }

      //! @brief Access scaling for the conjugate search direction, i.e. \f$\frac{(r,Pr)}{(\delta x,A\delta x)}\f$
      double alpha() const
      {
        return cache_->alpha;
      }

      //! @brief Access length of conjugate search direction with respect to the energy norm, i.e. \f$(\delta x,A\delta x)\f$.
      double length() const
      {
        return cache_->dxAdx;
      }

      //! @brief Access norm of residual with respect to the norm induced by the preconditioner, i.e. \f$(r,Pr)\f$, where \f$r=b-Ax\f$.
      double preconditionedResidualNorm() const
      {
        return cache_->sigma;
      }

      //! @brief Access norm of residual with respect to the employed scalar product, i.e. \f$\|r\|\f$, where \f$r=b-Ax\f$.
      double residualNorm() const
      {
        return cache_->residualNorm;
      }

    protected:
      Cache* cache_;
   };

    //! Bind second template argument of CG::InterfaceImpl to satisfy the interface of GenericStep.
    template <class Domain, class Range>
    using Interface = InterfaceImpl< Cache<Domain,Range>, Name >;


    //! Apply preconditioner, possibly with iterative refinements.
    class ApplyPreconditioner
        : public Mixin::IterativeRefinements
    {
    public:
      template < class Cache >
      void operator()( Cache& cache ) const
      {
        cache.P->apply( cache.Pr, cache.r );

        if( iterativeRefinements() > 0 )
        {
          auto r2 = cache.r;
          auto dQr = cache.Pr;
          for(auto i=0u; i<iterativeRefinements(); ++i)
          {
            cache.A->applyscaleadd(-1,cache.Pr,r2);
            cache.P->apply(dQr,r2);
            cache.Pr += dQr;
          }
        }

        using std::abs;
        if( cache.sigma < 0 )
          cache.sigma = abs( cache.sp->dot( cache.r, cache.Pr ) );
        cache.residualNorm = cache.sp->norm( cache.r );
      }

      template <class Preconditioner, class Domain, class Range>
      void pre(Preconditioner& P, Domain& x, Range& b) const
      {
        P.pre(x,b);
      }

      template <class Preconditioner, class Domain>
      void post(Preconditioner& P, Domain& x) const
      {
        P.post(x);
      }
    };


    //! Compute search direction for the conjugate gradient method.
    class SearchDirection
    {
    public:
      template < class Cache >
      void operator()( Cache& cache) const
      {
        if( cache.firstStep )
        {
          cache.dx = cache.Pr;
          computeInducedStepLength(cache);
          cache.firstStep = false;
          return;
        }

        using std::abs;
        auto newSigma = abs( cache.sp->dot(cache.r,cache.Pr) );
        cache.beta = newSigma/cache.sigma;
        cache.dx *= cache.beta; cache.dx += cache.Pr;
        cache.sigma = newSigma;

        computeInducedStepLength(cache);
      }

    private:
      template < class Cache >
      void computeInducedStepLength( Cache& cache ) const
      {
        cache.A->apply(cache.dx,cache.Adx);
        cache.dxAdx = cache.sp->dot(cache.dx,cache.Adx);
      }
    };


    //! Compute scaling of the search direction for the conjugate gradient method.
    class Scaling
    {
    public:
      template < class Cache >
      void operator()( Cache& cache ) const
      {
        cache.alpha = cache.sigma/cache.dxAdx;
      }
    };


    class UpdateIterate
    {
    public:
      template < class Cache >
      void operator()( Cache& cache ) const
      {
        cache.x.axpy(cache.alpha,cache.dx);
        cache.r.axpy(-cache.alpha,cache.Adx);
      }
    };


    //! Step implementation for the conjugate gradient method.
    template <class Domain, class Range=Domain>
    using Step =
    GenericStep< Domain, Range,
      ApplyPreconditioner,
      SearchDirection,
      Scaling,
      UpdateIterate,
      Interface< Domain, Range >
    >;
  }


  /*!
    @ingroup ISTL_Solvers
    @brief Conjugate gradient method (see @cite Hestenes1952).

    Solves quadratic optimization problems of the form \f$ \frac{1}{2}x^T Ax - b^T x \f$, where \f$A:\ X\mapsto Y\f$ is a positive definite linear operator.

    @tparam Domain domain space \f$X\f$
    @tparam Range range space \f$Y\f$
    @tparam TerminationCriterion termination criterion (such as Dune::KrylovTerminationCriterion::ResidualBased or Dune::KrylovTerminationCriterion::RelativeEnergyError (default))
   */
  template <class Domain, class Range,
            template <class> class TerminationCriterion = KrylovTerminationCriterion::RelativeEnergyError>
  using MyCGSolver = GenericIterativeMethod< CGSpec::Step<Domain,Range> , TerminationCriterion< real_t<Domain> > >;


  /*!
    @ingroup ISTL_Solvers
    @brief Generate conjugate gradient method.

    Solves equations of the form \f$PAx=Pb\f$, where \f$A:\ X\mapsto Y\f$ is a linear operator and
    \f$ P:\ Y\mapsto X\f$ a preconditioner.

    Usage:
    @code{.cpp}
    auto cg   = make_cg<Dune::CG  ,Dune::KrylovTerminationCriterion::ResidualBased>(A,P,sp,...);
    auto rcg  = make_cg<Dune::RCG ,Dune::KrylovTerminationCriterion::ResidualBased>(A,P,sp,...);
    auto tcg  = make_cg<Dune::TCG ,Dune::KrylovTerminationCriterion::ResidualBased>(A,P,sp,...);
    auto trcg = make_cg<Dune::TRCG,Dune::KrylovTerminationCriterion::ResidualBased>(A,P,sp,...);
    @endcond

    @param A linear operator
    @param P preconditioner
    @param sp scalar product
    @param accuracy relative accuracy
    @param nSteps maximal number of steps
    @param verbosityLevel =1: print final statistics, =2: print information in each iteration
    @param eps maximal attainable accuracy
    @tparam CGType conjugate gradient variant (=CG,RCG,TCG or TRCG)
    @tparam TerminationCriterion termination criterion (such as Dune::KrylovTerminationCriterion::ResidualBased or Dune::KrylovTerminationCriterion::RelativeEnergyError)
    @tparam Domain domain space \f$X\f$
    @tparam Range range space \f$Y\f$
   */
  template <template <class,class,template <class> class> class CGType,
            template <class> class TerminationCriterion,
            class Domain, class Range, class real_type = real_t<Domain> >
  CGType<Domain,Range,TerminationCriterion> make_cg(LinearOperator<Domain,Range>& A,
                                                    Preconditioner<Domain,Range>& P,
                                                    ScalarProduct<Domain>& sp,
                                                    real_type accuracy = 1e-15, unsigned nSteps = 1000,
                                                    unsigned verbosityLevel = 0, real_type eps = 1e-15)
  {
    using MyCG = CGType<Domain,Range,TerminationCriterion>;
    TerminationCriterion< real_t<Domain> > terminationCriterion;
    terminationCriterion.setRelativeAccuracy(accuracy);
    terminationCriterion.setEps(eps);
    auto cg = MyCG{ typename MyCG::Step{ A,P,sp } , std::move(terminationCriterion) };
    cg.setMaxSteps(nSteps);
    cg.setVerbosityLevel(verbosityLevel);
    return cg;
  }
}

#endif // DUNE_CG_HH
