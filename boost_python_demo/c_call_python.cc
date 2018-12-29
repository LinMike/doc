#include <iostream>
#include <vector>
#include <python2.7/Python.h>
#include <boost/python.hpp>
#include <unistd.h>

using namespace std;
namespace py = boost::python;

template<class T>
py::list vector2pylist(const vector<T>& v)
{
	py::object get_iter = py::iterator<std::vector<T> >();
	py::object iter = get_iter(v);
	py::list l(iter);
	return l;
}

int main(void)
{
	Py_Initialize();
	if(!Py_IsInitialized())
	{
		cout<<"python initialized failed"<<endl;
		return -1;
	}
	//initialize thread support
//	PyEval_InitThreads();
	//before start thread, in order to release global lock from init threads, or thread can't get global lock
//	PyEval_ReleaseThread(PyThreadState_Get());

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("import matplotlib.pyplot as plt");
	PyRun_SimpleString("import numpy as np");
	PyRun_SimpleString("import ctypes");
	PyRun_SimpleString("import threading");
	PyRun_SimpleString("import thread");
	PyRun_SimpleString("import cv2");
	PyRun_SimpleString("sys.path.append('./')");
	py::object mainMoudule;
	py::object mainNamespace;

	try
	{
		cout<<"prepared to call python function"<<endl;
		mainMoudule = py::import("__main__");
		mainNamespace = mainMoudule.attr("__dict__");
		py::object simple = exec_file("py_demo.py", mainMoudule, mainNamespace);
//		py::object helloworld = mainNamespace["Hello"];
//		helloworld("aaa");
//		py::object showimg = mainNamespace["__main__"];
//		showimg();

//		py::object add = mainNamespace["Add"];
//		cout<<"get function add = "<<&add<<endl;
//		int val = py::extract<int>(add(2,4));
//		cout<<"call Add return = "<<val<<endl;
//
//
////		int a[]={1,2,3,4,5,7},b[]={2,4,6,9,13,15},c[]={2,3,6,7,8,11};
//		py::list temp, la, lb, lc;
//		la.append(1);
//		la.append(2);
//		la.append(3);
//		la.append(4);
//		la.append(5);
//		la.append(6);
////		cout<<"temp "<<py::extract<int>(temp.pop())<<endl;
////		la.append(temp);
////		la.append(temp);
//		lb.append(la);
//		lb.append(la);
//		lc.append(la);
//		lc.append(la);
////		cout<<"la "<<py::extract<int>(la.pop())<<endl;
////		std::vector<int> va(&a[0], &a[5]), vb(&b[0], &b[5]), vc(&c[0], &c[5]);
////		std::vector<std::vector<int> > la, lb, lc;
////		la.push_back(va);
////		la.push_back(vb);
////		lb.push_back(vb);
////		lb.push_back(vc);
////		lc.push_back(vc);
////		lc.push_back(va);
//
//		py::object show = mainNamespace["drawData"];
//
////		py::list la, lb, lc;
////		la.append(vector2pylist(va));
////		la.append(vector2pylist(vb));
////		cout<<"la "<<la.count(3)<<endl;
////		lb = la;
////		lc = la;
//
//		cout<<"before call show"<<endl;
//		cout<<"c++ current thread id = "<<getpid()<<endl;//boost::this_thread::get_id()
////		vector2pylist(a[0]);
//		show();
////		show(lb, lb, lc);
	}
	catch(...)
	{
		if(PyErr_Occurred())
		{
			PyErr_Print();
		}
	}
//	PyGILState_Ensure();
	Py_Finalize();

	return 0;
}
