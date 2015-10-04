/**
 * \file DaoFunction.h
 * \author Klaus Beyer
 * \date 10 September 2010
 * \brief This file implements the interface to call a routine in a dao script
 * \details The template class has the return type as type parameter. The method call() 
 * takes either the arguments in a tuple or as a single argument. Even no arguments
 * are supported. Currently supported argument and return types are  
 * int, long, float, double, const char *. If an unsupported type is given and the 
 * compiler doesn't catch it an exception is thrown. Here are some examples.
 * \code
 * //routine daoroutine()
 * DaoFunctionCall<> simple("daoroutine");
 * simple();
 * 
 * //routine daoroutine(a : int , b : float , c : string) => int
 * DaoFunctionCall<int> routine("daoroutine");
 * tuple<int,float,const char*> args(1,2.5f, "abc");
 * int ret = routine(args);
 * // or 
 * int ret = DaoFunctionCall<int> ("daoroutine")(make_tuple(1,2.5f, "abc"));
 * 
 * // routine daoroutine(a : int) => double
 * DaoFunctionCall<double> routine("daoroutine");
 * double v = routine(100);
 * \endcode
 **/
#pragma once
#include<dao.h>
#include <vector>
#include <memory>
#include <exception>
#include "DaoVm.h"
#include <type_traits>
#include <tuple>
#include "main/Exception.h"
#include "logging/logger.h"

namespace FTS {
class DaoVmFunctionNotFound : public NotExistException 
{
public:
    DaoVmFunctionNotFound(const String& in_sObjectName, const MsgType::Enum& in_gravity = MsgType::Error) throw()
    : LoggableException(new I18nLoggerCmd("DaoVmFunctionNotExist", in_gravity, in_sObjectName))
    {

    } ;
};
class DaoVmInvalidDataType : public InvalidCallException 
{
public:
    DaoVmInvalidDataType(const String& in_sInfo) 
        : LoggableException(new I18nLoggerCmd("InvType", MsgType::Horror, "InvalidDataType in DaoFunctionCall being used!"))
    {};

};


template<typename tuple_type, typename F, int Index, int Max>
struct foreach_tuple_impl {
    void operator()(tuple_type & t, F& f) {
        f.alloc(std::get<Index>(t));
        foreach_tuple_impl<tuple_type, F, Index + 1, Max>()(t, f);
    }
};

template<typename tuple_type, typename F, int Max>
struct foreach_tuple_impl<tuple_type, F, Max, Max> {
    void operator()(tuple_type & t, F& f) {
        f.alloc(std::get<Max>(t));
    }
};

template<typename tuple_type, typename F>
void foreach_tuple_element(tuple_type & t, F& f)
{
    foreach_tuple_impl<tuple_type, F, 0, std::tuple_size<tuple_type>::value - 1>()(t, f);
}

template<class T> 
T getDValue(DValue d) { throw DaoVmInvalidDataType("Return type parameter not supported");}

template<> 
int getDValue(DValue d);
template<> 
long getDValue(DValue d); 
template<> 
String getDValue(DValue d) ;
template<> 
float getDValue(DValue d) ;
template<> 
double getDValue(DValue d);


template<class RetType = int>
class DaoFunctionCall
{
public:
    DaoFunctionCall(const char * name) : _name(name) {}
    DaoFunctionCall(const String& name) : _name(name.c_str()) {}
    virtual ~DaoFunctionCall(void) {};
    
    RetType operator()()
    {
        DaoVm::getSingleton().execRoutine(getName(), nullptr, 0);
        DValue ret = DaoVm::getSingleton().getReturn();
        setReturn( getDValue<RetType>(ret) );
        return _ret;
    }
    template<class tuple_type>
    RetType operator()(tuple_type arg,
    typename std::enable_if< !std::is_integral<tuple_type>::value >::type * dummy = 0 )
    {
        int count = std::tuple_size<tuple_type>::value ;
        DValue **par = nullptr ;

        if( count > 0 ) {
            foreach_tuple_element(arg, *this);
            par = new DValue*[count];
            for(int i=0; i<count; i++) {
                par[i] = getParameter(i) ;
            }
        }
        DaoVm::getSingleton().execRoutine(getName(), par, count);
        DValue ret = DaoVm::getSingleton().getReturn();
        setReturn( getDValue<RetType>(ret) );
        for(int i=0; i<count; i++) {
            DValue_Clear(par[i]);
        }
        delete [] par ;
        return _ret;
    }
    template<class arg_type>
    RetType operator()(arg_type arg, 
        typename std::enable_if< std::is_integral<arg_type>::value >::type * dummy = 0 )
    {
        DValue* par = new DValue ;
        *par = daoNew(arg) ;
        DaoVm::getSingleton().execRoutine(getName(), &par, 1);
        DValue ret = DaoVm::getSingleton().getReturn();
        setReturn( getDValue<RetType>(ret) );
        DValue_Clear(par);
        delete par;
        return _ret;
    }
    RetType operator()(const char * arg)
    {
        DValue* par = new DValue ;
        *par = daoNew(arg) ;
        DaoVm::getSingleton().execRoutine(getName(), &par, 1);
        DValue ret = DaoVm::getSingleton().getReturn();
        setReturn( getDValue<RetType>(ret) );
        DValue_Clear(par);
        delete par;
        return _ret;
    }
    RetType operator()(const String& arg)
    {
        return this->operator()(arg.c_str());
    }
    template<typename T>
    void alloc(const T & t)
    {

        DValue * par = new DValue;
        *par = daoNew(t);
        add(par);
    }
    
private:
    void add(DValue* val) {_param.push_back(val);}
    DValue daoNew(int param)
    {
        return DValue_NewInteger(param);
    }

    DValue daoNew(float param)
    {
        return DValue_NewFloat(param);
    }
    DValue daoNew(double param)
    {
        return DValue_NewDouble(param);
    }

    DValue daoNew(const char * param)
    {
        return DValue_NewMBString(const_cast<char *>(param), 0);
    }
    DValue daoNew(const String& param)
    {
        return DValue_NewMBString(const_cast<char *>(param.c_str()), 0);
    }
    void setReturn(RetType ret) {_ret = ret;}
    RetType getReturnValue() {return _ret;}
    const char * getName() {return _name;}
    int getParameterCount() {return _param.size();}
    DValue* getParameter(int index) {return _param[index];}
    const char * _name;
    RetType _ret ;
    std::vector<DValue*> _param;
};

} // Namespace FTS
