#include <vector>

/**
 * @brief Return combinations of the length of two of elements.
 *
 * combinationsOfTwo("ABCD") --> AB AC AD BC BD CD
 *
 * Elements are treated as unique based on their position, not on their value.
 * So if the input elements are unique, there will be no repeat values in each
 * combination.
 *
 * @tparam T the type of each element.
 * @param elements combine these elements.
 *
 * @return a vector of pairs of all combinations.
 */
template <typename T>
std::vector<std::pair<T, T> > combinationsOfTwo(const std::vector<T>& elements)
{
	std::vector<std::pair<T, T> > result;

	typename std::vector<T>::const_iterator elementA;
	for (elementA = elements.begin(); elementA != elements.end(); elementA++) {

		typename std::vector<T>::const_iterator elementB;
		for (elementB = elementA; elementB != elements.end(); elementB++) {

			// don't use the same elements
			if (elementB == elementA) {
				continue;
			}

			// add the combiantion
			result.push_back(std::pair<T, T>(*elementA, *elementB));
		}
	}
	return result;
}
