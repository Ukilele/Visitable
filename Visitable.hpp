#pragma once

#include "boost/mpl/at.hpp"
#include "boost/mpl/begin_end.hpp"
#include "boost/mpl/contains.hpp"
#include "boost/mpl/deref.hpp"
#include "boost/mpl/index_of.hpp"
#include "boost/mpl/is_sequence.hpp"
#include "boost/mpl/next_prior.hpp"
#include "boost/mpl/placeholders.hpp"
#include "boost/mpl/size.hpp"
#include "boost/mpl/transform.hpp"
#include "boost/mpl/vector.hpp"
#include "boost/type_traits/add_const.hpp"
#include "boost/utility/declval.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>


namespace misc
{
	namespace detail
	{
		template<class Visitor, class AHierarchyType, class VisitableStorageType>
		inline auto call_impl(Visitor&& vis, VisitableStorageType storage)
			-> decltype(std::forward<Visitor>(vis)(*static_cast<AHierarchyType*>(storage)))
		{
			return std::forward<Visitor>(vis)(*static_cast<AHierarchyType*>(storage));
		}

		template<class Visitor, class Iter, class End, class VisitableStorageType, std::size_t Index>
		struct CallImplArrayFiller
		{
			template<class T, std::size_t N>
			static void Fill(T(&arr)[N])
			{
				typedef typename boost::mpl::deref<Iter>::type current_type;
				arr[Index] = &call_impl<Visitor, current_type, VisitableStorageType>;
				typedef typename boost::mpl::next<Iter>::type next_iter;
				CallImplArrayFiller<Visitor, next_iter, End, VisitableStorageType, Index + 1>::Fill(arr);
			}
		};

		template<class Visitor, class EndIter, class VisitableStorageType, std::size_t Index>
		struct CallImplArrayFiller<Visitor, EndIter, EndIter, VisitableStorageType, Index>
		{
			template<class T, std::size_t N>
			static void Fill(T(&)[N])
			{
				static_assert(N == Index, "The size of the array is not equal to the size of the sequence.");
			}
		};

		template<class Visitor, class AFirstHierarchyType, class ASecondHierarchyType, class FirstVisitableStorageType, class SecondVisitableStorageType>
		inline auto call_impl2D(Visitor&& vis, FirstVisitableStorageType firstStorage, SecondVisitableStorageType secondStorage)
			-> decltype(std::forward<Visitor>(vis)(*static_cast<AFirstHierarchyType*>(firstStorage), *static_cast<ASecondHierarchyType*>(secondStorage)))
		{
			return std::forward<Visitor>(vis)(*static_cast<AFirstHierarchyType*>(firstStorage), *static_cast<ASecondHierarchyType*>(secondStorage));
		}

		template<class Visitor, class FirstIter, class FirstEndIter, class SecondBeginIter, class SecondIter, class SecondEndIter, class FirstStorageType, class SecondStorageType, std::size_t Index>
		struct CallImpl2DArrayFiller
		{
			template<class T, std::size_t N>
			static void Fill(T(&arr)[N])
			{
				typedef typename boost::mpl::deref<FirstIter>::type first_type;
				typedef typename boost::mpl::deref<SecondIter>::type second_type;
				arr[Index] = &call_impl2D<Visitor, first_type, second_type, FirstStorageType, SecondStorageType>;
				typedef typename boost::mpl::next<SecondIter>::type second_iter_next;
				CallImpl2DArrayFiller<Visitor, FirstIter, FirstEndIter, SecondBeginIter, second_iter_next, SecondEndIter, FirstStorageType, SecondStorageType, Index + 1>::Fill(arr);
			}
		};

		//First specialization, when SecondIter reaches to its end
		template<class Visitor, class FirstIter, class FirstEndIter, class SecondBeginIter, /*class SecondIter, */class SecondEndIter, class FirstStorageType, class SecondStorageType, std::size_t Index>
		struct CallImpl2DArrayFiller<Visitor, FirstIter, FirstEndIter, SecondBeginIter, SecondEndIter, SecondEndIter, FirstStorageType, SecondStorageType, Index>
		{
			template<class T, std::size_t N>
			static void Fill(T(&arr)[N])
			{
				typedef typename boost::mpl::next<FirstIter>::type first_iter_next;
				CallImpl2DArrayFiller<Visitor, first_iter_next, FirstEndIter, SecondBeginIter, SecondBeginIter, SecondEndIter, FirstStorageType, SecondStorageType, Index>::Fill(arr);
			}
		};

		//Second spezialication, when FirstIter reaches to its end
		template<class Visitor, /*class FirstIter, */class FirstEndIter, class SecondBeginIter, class SecondIter, class SecondEndIter, class FirstStorageType, class SecondStorageType, std::size_t Index>
		struct CallImpl2DArrayFiller<Visitor, FirstEndIter, FirstEndIter, SecondBeginIter, SecondIter, SecondEndIter, FirstStorageType, SecondStorageType, Index>
		{
			template<class T, std::size_t N>
			static void Fill(T(&)[N])
			{
				static_assert(N == Index, "The size of the array is not equal to the size of the sequences.");
			}
		};

		template<class Visitable>
		struct const_propagated_visitable_types
		{
			typedef typename Visitable::visitable_types type;
		};

		template<class Visitable>
		struct const_propagated_visitable_types<const Visitable>
		{
			typedef typename boost::mpl::transform<
				typename Visitable::visitable_types,
				boost::add_const<boost::mpl::placeholders::_1>
			>::type
				type;
		};

		template<class Visitable>
		struct const_propagated_visitable_types<Visitable&>
		{
			typedef typename const_propagated_visitable_types<Visitable>::type type;
		};

		template<class Visitable>
		std::size_t index(const Visitable& v) { return v.m_hierarchyTypeIndex; }

		template<class Hierarchy, class Visitor, class FirstVisitable>
		inline auto visit_impl(Visitor&& visitor, FirstVisitable&& first)
			-> decltype(
				std::forward<Visitor>(visitor)(boost::declval<typename boost::mpl::at_c<Hierarchy, 0>::type&>())
				)
		{
			typedef typename boost::mpl::at_c<Hierarchy, 0>::type first_type;
			typedef decltype(std::forward<Visitor>(visitor)(boost::declval<first_type&>())) result_type;
			const bool isConst = std::is_const<typename std::remove_reference<FirstVisitable>::type>::value;
			typedef typename std::conditional<isConst, void const*, void*>::type visitable_storage_type;
			typedef result_type(*fptr_type)(Visitor&&, visitable_storage_type);

			fptr_type functionArray[boost::mpl::size<Hierarchy>::value];
			typedef typename boost::mpl::begin<Hierarchy>::type begin_iter;
			typedef typename boost::mpl::end<Hierarchy>::type end_iter;
			CallImplArrayFiller<Visitor, begin_iter, end_iter, visitable_storage_type, 0>::Fill(functionArray);
			return functionArray[index(first)](std::forward<Visitor>(visitor), &first);
		}

		template<class FirstHierarchy, class SecondHierarchy, class Visitor, class FirstVisitable, class SecondVisitable>
		inline auto visit_impl(Visitor&& visitor, FirstVisitable&& first, SecondVisitable&& second)
			-> decltype(
				std::forward<Visitor>(visitor)(
					boost::declval<typename boost::mpl::at_c<FirstHierarchy, 0>::type&>(),
					boost::declval<typename boost::mpl::at_c<SecondHierarchy, 0>::type&>()
					)
				)
		{
			typedef typename boost::mpl::at_c<FirstHierarchy, 0>::type first_type_from_first_hierarchy;
			typedef typename boost::mpl::at_c<SecondHierarchy, 0>::type first_type_from_second_hierarchy;
			typedef decltype(std::forward<Visitor>(visitor)(boost::declval<first_type_from_first_hierarchy&>(), boost::declval<first_type_from_second_hierarchy&>())) result_type;
			const bool isFirstConst = std::is_const<typename std::remove_reference<FirstVisitable>::type>::value;
			const bool isSecondConst = std::is_const<typename std::remove_reference<SecondVisitable>::type>::value;
			typedef typename std::conditional<isFirstConst, void const*, void*>::type first_visitable_storage_type;
			typedef typename std::conditional<isSecondConst, void const*, void*>::type second_visitable_storage_type;
			typedef result_type(*fptr_type)(Visitor&&, first_visitable_storage_type, second_visitable_storage_type);

			const auto firstHierarchySize = boost::mpl::size<FirstHierarchy>::value;
			const auto secondHierarchySize = boost::mpl::size<SecondHierarchy>::value;
			fptr_type functionArray[firstHierarchySize * secondHierarchySize];
			typedef typename boost::mpl::begin<FirstHierarchy>::type first_begin_iter;
			typedef typename boost::mpl::end<FirstHierarchy>::type first_end_iter;
			typedef typename boost::mpl::begin<SecondHierarchy>::type second_begin_iter;
			typedef typename boost::mpl::end<SecondHierarchy>::type second_end_iter;
			CallImpl2DArrayFiller<Visitor, first_begin_iter, first_end_iter, second_begin_iter, second_begin_iter, second_end_iter, first_visitable_storage_type, second_visitable_storage_type, 0>::Fill(functionArray);
			const auto overallIndex = index(first) * secondHierarchySize + index(second);
			return functionArray[overallIndex](std::forward<Visitor>(visitor), &first, &second);
		}

	}

	//Forward-declare this function as it is called by the legacy-Accept-functions
	template<class Visitor, class FirstVisitable>
	inline auto visit(Visitor&& visitor, FirstVisitable&& first) -> decltype(
			detail::visit_impl<typename detail::const_propagated_visitable_types<FirstVisitable>::type>(std::forward<Visitor>(visitor), std::forward<FirstVisitable>(first))
			);


	//Visitable API:

	template<class T>
	class type
	{
		type() {} //Make the constructor private
		friend T; //Only T is able to construct type<T>
	};

	template<class VisitableTypes>
	class Visitable
	{
		static_assert(boost::mpl::is_sequence<VisitableTypes>::value, "VisitableTypes must be a sequence (e.g. boost::mpl::vector).");
		static_assert(boost::mpl::size<VisitableTypes>::value > 0, "You cannot instantiate Visitable without any visitable types.");

	public:
		typedef VisitableTypes visitable_types;

		//Legacy-Code

		template<class LegacyVisitor>
		void AcceptVisitor(LegacyVisitor&& v) { visit(v, *this); }

		template<class LegacyVisitor>
		void AcceptVisitor(LegacyVisitor&& v) const { visit(v, *this); }

		template<class LegacyVisitor>
		void AcceptVirtual(LegacyVisitor&& v) { visit(v, *this); }

		template<class LegacyVisitor>
		void AcceptVirtual(LegacyVisitor&& v) const { visit(v, *this); }

	protected:
		template<class ActualType>
		Visitable(type<ActualType>)
		{
			static_assert(boost::mpl::contains<visitable_types, ActualType>::value, "Specified type is not within the visitable types.");
			m_hierarchyTypeIndex = typename boost::mpl::index_of<visitable_types, ActualType>::type();
		}

	private:
		std::size_t m_hierarchyTypeIndex;

		template<class V>
		friend std::size_t detail::index(const V&);
	};


	template<class Visitor, class FirstVisitable>
	inline auto visit(Visitor&& visitor, FirstVisitable&& first) -> decltype(
			detail::visit_impl<typename detail::const_propagated_visitable_types<FirstVisitable>::type>(std::forward<Visitor>(visitor), std::forward<FirstVisitable>(first))
			)
	{
		return detail::visit_impl<typename detail::const_propagated_visitable_types<FirstVisitable>::type>(std::forward<Visitor>(visitor), std::forward<FirstVisitable>(first));
	}

	template<class Visitor, class FirstVisitable, class SecondVisitable>
	inline auto visit(Visitor&& visitor, FirstVisitable&& first, SecondVisitable&& second) -> decltype(
			detail::visit_impl<typename detail::const_propagated_visitable_types<FirstVisitable>::type,
			typename detail::const_propagated_visitable_types<SecondVisitable>::type
			>(std::forward<Visitor>(visitor), std::forward<FirstVisitable>(first), std::forward<SecondVisitable>(second))
			)
	{
		return detail::visit_impl<typename detail::const_propagated_visitable_types<FirstVisitable>::type,
			typename detail::const_propagated_visitable_types<SecondVisitable>::type
		>(std::forward<Visitor>(visitor), std::forward<FirstVisitable>(first), std::forward<SecondVisitable>(second));
	}
}

//Or maybe just use: variant<...> ?