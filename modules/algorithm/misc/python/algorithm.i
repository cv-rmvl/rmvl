%module rm_algorithm

%{
#include "rmvl/algorithm/numcal.hpp"
#include "rmvl/algorithm/datastruct.hpp"
%}

%include "std_vector.i"
%include "std_pair.i"

%include "build_algorithm_def.i"
%include "rm_numcal.i"

%include "../../include/rmvl/algorithm/datastruct.hpp"

%template (RaHeapInt) rm::RaHeap<int>;
%template (RaHeapFloat) rm::RaHeap<float>;
%template (RaHeapDouble) rm::RaHeap<double>;

 