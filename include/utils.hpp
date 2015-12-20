#ifndef FR_BENOU_UTILS_H_
#define FR_BENOU_UTILS_H_

#include <ostream>
#include <utility>

template <class T1, class T2>
std::ostream& operator<< (std::ostream& out, const std::pair<T1, T2>& val)
{
    out << "(" << val.first << ", " << val.second << ")";
    return out;
}

#endif  /* FR_BENOU_UTILS_H_ */
