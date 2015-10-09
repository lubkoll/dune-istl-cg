#ifndef DUNE_MIXIN_ABSOLUTE_ACCURACY_HH
#define DUNE_MIXIN_ABSOLUTE_ACCURACY_HH

#include <cassert>
#include <limits>

namespace Dune
{
  namespace Mixin
  {
    /**
     * @ingroup MixinGroup
     * @brief %Mixin class for absolute accuracy.
     */
    template <class real_type=double>
    class AbsoluteAccuracy
    {
    public:
      /**
       * @brief Constructor.
       * @param accuracy absolute accuracy
       */
      explicit AbsoluteAccuracy(real_type accuracy = std::numeric_limits<real_type>::epsilon())
        : absoluteAccuracy_{accuracy}
      {
        assert(absoluteAccuracy_ >= 0);
      }

      /**
       * @brief Set absolute accuracy.
       * @param accuracy absolute accuracy
       */
      void setAbsoluteAccuracy(real_type accuracy)
      {
        absoluteAccuracy_ = accuracy;
        assert(absoluteAccuracy_ >= 0);
      }

      /**
       * @brief Access absolute accuracy.
       * @return absolute accuracy
       */
      real_type absoluteAccuracy() const
      {
        return absoluteAccuracy_;
      }

    private:
      real_type absoluteAccuracy_;
    };
  }
}

#endif // DUNE_MIXIN_ABSOLUTE_ACCURACY_HH
