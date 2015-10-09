/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*   This file is part of the C++-library FunG.                              */
/*   Copyright 2015 Lars Lubkoll                                               */
/*                                                                             */
/*   FunG is free software: you can redistribute it and/or modify            */
/*   it under the terms of the GNU General Public License as published by      */
/*   the Free Software Foundation, either version 3 of the License, or         */
/*   (at your option) any later version.                                       */
/*                                                                             */
/*   FunG is distributed in the hope that it will be useful,                 */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*   GNU General Public License for more details.                              */
/*                                                                             */
/*   You should have received a copy of the GNU General Public License         */
/*   along with FunG.  If not, see <http://www.gnu.org/licenses/>.           */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DUNE_UTIL_VOIDER_HH
#define DUNE_UTIL_VOIDER_HH

namespace Dune
{
  /**
   * \cond DOCUMENT_IMPLEMENTATION_DETAILS
   */
  namespace GenericIterativeMethodDetail
  {
    /// helper to make gcc happy
    template <class...> struct voider { using type = void; };
  }
  /**
   * \endcond
   */

  /// Most fascinating type ever. Is void for all input types.
  template <class... Types>
  using void_t = typename GenericIterativeMethodDetail::voider<Types...>::type;
}

#endif // DUNE_UTIL_VOIDER_HH
