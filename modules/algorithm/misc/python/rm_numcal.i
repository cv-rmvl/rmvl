%template (vector_i) std::vector<int>;
%template (vector_f) std::vector<float>;
%template (vector_d) std::vector<double>;
%template (vector_vd) std::vector<std::vector<double>>;
%template (pair_d_d) std::pair<double, double>;
%template (pair_vd_d) std::pair<std::vector<double>, double>;

%{
// Python List -> std::vector<double>
static std::vector<double> pylist_to_vd(PyObject* input) {
    std::vector<double> retval;
    if (!PyList_Check(input)) {
        PyErr_SetString(PyExc_TypeError, "Expecting a list: 'List<Float>'");
        return retval;
    }
    retval.resize(PyList_Size(input));
    for (std::size_t i = 0; i < retval.size(); i++) {
        PyObject *o = PyList_GetItem(input, i);
        if (!PyFloat_Check(o) && !PyLong_Check(o)) {
            PyErr_SetString(PyExc_TypeError, "List items must be numbers");
            return std::vector<double>();
        }
        retval[i] = PyFloat_AsDouble(o);
    }
    return retval;
}

// Python List -> std::vector<std::vector<double>>
static std::vector<std::vector<double>> pylist_to_vvd(PyObject* input) {
    std::vector<std::vector<double>> retval;
    if (!PyList_Check(input)) {
        PyErr_SetString(PyExc_TypeError, "Expecting a list: 'List<List<Float>>'");
        return retval;
    }
    retval.resize(PyList_Size(input));
    for (std::size_t i = 0; i < retval.size(); i++) {
        PyObject *o = PyList_GetItem(input, i);
        if (!PyList_Check(o)) {
            PyErr_SetString(PyExc_TypeError, "List items must be list: 'List<Float>'");
            return std::vector<std::vector<double>>();
        }
        retval[i] = pylist_to_vd(o);
    }
    return retval;
}

// std::vector<double> -> Python List
static PyObject* vd_to_pylist(const std::vector<double> &vec) {
    PyObject *pylist = PyList_New(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i) {
        PyList_SetItem(pylist, i, PyFloat_FromDouble(vec[i]));
    }
    return pylist;
}

// // std::vector<std::vector<double>> -> Python List
// static PyObject* vvd_to_pylist(const std::vector<std::vector<double>> &vec) {
//     PyObject *pylist = PyList_New(vec.size());
//     for (std::size_t i = 0; i < vec.size(); ++i) {
//         PyList_SetItem(pylist, i, vd_to_pylist(vec[i]));
//     }
//     return pylist;
// }

// Python Callable -> std::function<double(double)> {aka rm::Func1d}
static std::function<double(double)> pycallable_to_func1d(PyObject* pyfunc) {
    return [pyfunc](double x) -> double {
        PyObject *arglist = Py_BuildValue("(d)", x);
        PyObject *result = PyObject_CallObject(pyfunc, arglist);
        Py_DECREF(arglist);
        double retval{};
        if (result != nullptr) {
            retval = PyFloat_AsDouble(result);
            Py_DECREF(result);
            if (PyErr_Occurred()) {
                PyErr_Print();
                retval = 0.0;
            }
        } else {
            PyErr_Print();
        }
        return retval;
    };
}

// // Python Callable -> std::vector<rm::Func1d> {aka rm::Func1ds}
// static std::vector<rm::Func1d> pylist_to_func1ds(PyObject* input) {
//     std::vector<rm::Func1d> retval;
//     if (!PyList_Check(input)) {
//         PyErr_SetString(PyExc_TypeError, "Expecting a list: 'List<Func1d>'");
//         return retval;
//     }
//     retval.resize(PyList_Size(input));
//     for (std::size_t i = 0; i < retval.size(); i++) {
//         PyObject *o = PyList_GetItem(input, i);
//         if (!PyCallable_Check(o)) {
//             PyErr_SetString(PyExc_TypeError, "List items must be callable: 'Func1d'");
//             return std::vector<rm::Func1d>();
//         }
//         retval[i] = pycallable_to_func1d(o);
//     }
//     return retval;
// }

// Python Callable -> std::function<double(const std::vector<double>&)> {aka rm::FuncNd}
static std::function<double(const std::vector<double>&)> pycallable_to_funcnd(PyObject* pyfunc) {
    return [pyfunc](const std::vector<double>& x) -> double {
        PyObject *pylist = vd_to_pylist(x);
        PyObject *arglist = Py_BuildValue("(O)", pylist);
        PyObject *result = PyObject_CallObject(pyfunc, arglist);
        Py_DECREF(arglist);
        Py_DECREF(pylist);
        double retval{};
        if (result != nullptr) {
            retval = PyFloat_AsDouble(result);
            Py_DECREF(result);
            if (PyErr_Occurred()) {
                PyErr_Print();
                retval = 0.0;
            }
        } else {
            PyErr_Print();
        }
        return retval;
    };
}

// Python Callable -> std::vector<rm::FuncNd> {aka rm::FuncNds}
static std::vector<rm::FuncNd> pylist_to_funcnds(PyObject* input) {
    std::vector<rm::FuncNd> retval;
    if (!PyList_Check(input)) {
        PyErr_SetString(PyExc_TypeError, "Expecting a list: 'List<FuncNd>'");
        return retval;
    }
    retval.resize(PyList_Size(input));
    for (std::size_t i = 0; i < retval.size(); i++) {
        PyObject *o = PyList_GetItem(input, i);
        if (!PyCallable_Check(o)) {
            PyErr_SetString(PyExc_TypeError, "List items must be callable: 'FuncNd'");
            return std::vector<rm::FuncNd>();
        }
        retval[i] = pycallable_to_funcnd(o);
    }
    return retval;
}

// Python Callable -> std::function<double(double, const std::vector<double> &)> {aka rm::Ode}
static std::function<double(double, const std::vector<double> &)> pycallable_to_ode(PyObject* pyfunc) {
    return [pyfunc](double x, const std::vector<double>& vec) -> double {
        PyObject *pylist = vd_to_pylist(vec);
        PyObject *arglist = Py_BuildValue("(dO)", x, pylist);
        PyObject *result = PyObject_CallObject(pyfunc, arglist);
        Py_DECREF(arglist);
        Py_DECREF(pylist);
        double retval{};
        if (result != nullptr) {
            retval = PyFloat_AsDouble(result);
            Py_DECREF(result);
            if (PyErr_Occurred()) {
                PyErr_Print();
                retval = 0.0;
            }
        } else {
            PyErr_Print();
        }
        return retval;
    };
}

// Python Callable -> std::vector<rm::Ode> {aka rm::Odes}
static std::vector<rm::Ode> pylist_to_odes(PyObject* input) {
    std::vector<rm::Ode> retval;
    if (!PyList_Check(input)) {
        PyErr_SetString(PyExc_TypeError, "Expecting a list: 'List<Ode>'");
        return retval;
    }
    retval.resize(PyList_Size(input));
    for (std::size_t i = 0; i < retval.size(); i++) {
        PyObject *o = PyList_GetItem(input, i);
        if (!PyCallable_Check(o)) {
            PyErr_SetString(PyExc_TypeError, "List items must be callable: 'Ode'");
            return std::vector<rm::Ode>();
        }
        retval[i] = pycallable_to_ode(o);
    }
    return retval;
}

%}

// Typemaps in normal functions
%typemap(in) std::function<double(double)> { $1 = pycallable_to_func1d($input); }
%apply std::function<double(double)> { rm::Func1d };
%typemap(in) std::function<double(const std::vector<double>&)> { $1 = pycallable_to_funcnd($input); }
%apply std::function<double(const std::vector<double>&)> { rm::FuncNd };
%typemap(in) std::vector<rm::Func1d> { $1 = pylist_to_func1ds($input); }
%apply std::vector<rm::Func1d> { rm::Func1ds };
%typemap(in) std::vector<rm::FuncNd> { $1 = pylist_to_funcnds($input); }
%apply std::vector<rm::FuncNd> { rm::FuncNds };

%typemap(typecheck) std::function<double(double)>, rm::Func1d { $1 = PyCallable_Check($input) ? 1 : 0; }
%typemap(typecheck) std::function<double(const std::vector<double>&)>, rm::FuncNd { $1 = PyCallable_Check($input) ? 1 : 0; }
%typemap(typecheck) std::vector<rm::Func1d>, rm::Func1ds { $1 = PyList_Check($input) ? 1 : 0; }
%typemap(typecheck) std::vector<rm::FuncNd>, rm::FuncNds { $1 = PyList_Check($input) ? 1 : 0; }

namespace rm {

// Ignore some features in Mordern C++ code
%ignore RungeKutta::init(double, std::vector<double> &&);

}

%include "../../include/rmvl/algorithm/numcal.hpp"

// Ignore and extend in class constructors
namespace rm {

%extend Polynomial {
    Polynomial(PyObject* coeffs) { return new rm::Polynomial(pylist_to_vd(coeffs)); }
}

%extend Interpolator {
    Interpolator(PyObject* xs, PyObject* ys) { return new rm::Interpolator(pylist_to_vd(xs), pylist_to_vd(ys)); }
}

%ignore CurveFitter::CurveFitter(const std::vector<double> &, const std::vector<double> &, std::bitset<8>);
%extend CurveFitter {
    CurveFitter(PyObject* xs, PyObject* ys, unsigned char order) { return new rm::CurveFitter(pylist_to_vd(xs), pylist_to_vd(ys), order); }
}

%ignore NonlinearSolver::NonlinearSolver(const std::function<double(double)> &);
%extend NonlinearSolver {
    NonlinearSolver(PyObject* pyfunc) { return new rm::NonlinearSolver(pycallable_to_func1d(pyfunc)); }
}

%ignore RungeKutta::RungeKutta(const Odes &fs, const std::vector<double> &p, const std::vector<double> &lambda, const std::vector<std::vector<double>> &r);
%extend RungeKutta {
    RungeKutta(PyObject* fs, PyObject* p, PyObject* lambda, PyObject* r) { return new rm::RungeKutta(pylist_to_odes(fs), pylist_to_vd(p), pylist_to_vd(lambda), pylist_to_vvd(r)); }
}

%ignore RungeKutta::init(double, const std::vector<double> &);
%extend RungeKutta {
    void init(double t, PyObject* x) { $self->init(t, pylist_to_vd(x)); }
}

%ignore RungeKutta2::RungeKutta2(const Odes &fs);
%extend RungeKutta2 {
    RungeKutta2(PyObject* fs) { return new rm::RungeKutta2(pylist_to_odes(fs)); }
}

%ignore RungeKutta3::RungeKutta3(const Odes &fs);
%extend RungeKutta3 {
    RungeKutta3(PyObject* fs) { return new rm::RungeKutta3(pylist_to_odes(fs)); }
}

%ignore RungeKutta4::RungeKutta4(const Odes &fs);
%extend RungeKutta4 {
    RungeKutta4(PyObject* fs) { return new rm::RungeKutta4(pylist_to_odes(fs)); }
}

}