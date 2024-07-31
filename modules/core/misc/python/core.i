%module rm_core

%{
#include "rmvl/core/version.hpp"
#include "rmvl/core/dataio.hpp"
#include "rmvl/core/timer.hpp"
%}

%rename (version) getVersionString;

%include "rmvl_version.hpp"
%include "../../include/rmvl/core/dataio.hpp"
%include "../../include/rmvl/core/timer.hpp"
