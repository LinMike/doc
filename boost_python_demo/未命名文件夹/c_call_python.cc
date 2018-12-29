#include <iostream>
#include <boost/python.hpp>

using namespace std;
namespace boost::python = py;

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
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");
	py::object mainMoudule;
	py::object mainNamespace;
	
	try
	{
		mainMoudule = import("__main__");
		mainNamespace = mainMoudule.attr("__dict__");
		py::object simple = exec_file("py_demo.py", mainMoudule, mainNamespace);
		py::object helloworld = mainNamespace["Hello"];
		helloworld("aaa");
		py::object add = mainNamespace["Add"];
		cout<<"get function add = "<<&add<<endl;
		int val = py::extract<int>(add(2,4));
		cout<<"call Add return = "<<val<<endl;
		
		
		std::vector<std::vector<int> > a,b,c;
		a[0].push_back(1);
		a[0].push_back(2);
		a[0].push_back(3);
		a[1].push_back(3);
		a[1].push_back(4);
		a[1].push_back(5);
		b[0].push_back(2);
		b[0].push_back(7);
		b[0].push_back(8);
		b[1].push_back(3);
		b[1].push_back(4);
		b[1].push_back(5);
		c[0].push_back(2);
		c[0].push_back(5);
		c[0].push_back(7);
		c[1].push_back(3);
		c[1].push_back(4);
		c[1].push_back(5);


		py::object show = mainNamespace["drawData"];
		py::list la;
		vector2pylist(a[0]);
		show();
	}
	catch(...)
	{
		if(PyErr_Occurred())
		{
			PyErr_Print();
		}
	}
	Py_Finalize();

	return 0;
}
