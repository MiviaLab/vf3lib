#ifndef STACK_HPP
#define STACK_HPP

namespace vflib
{

template<typename T >
class Stack
{
    public:
        virtual void push(T const& data)=0;
        virtual size_t size()=0;
        virtual std::shared_ptr<T > pop()=0;
};

}

#endif