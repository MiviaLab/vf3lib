#ifndef SYNCHRONIZEDSTACK_HPP
#define SYNCHRONIZEDSTACK_HPP

#include <mutex>
#include <stack>
#include "Stack.hpp"

namespace vflib
{

template<typename T >
class SynchronizedStack : public Stack<T >
{
    private:
        std::stack<std::shared_ptr<T >> stack;
        std::mutex mutex;

    public:
        void push(T const& data){
            std::lock_guard<std::mutex> guard(mutex);
            stack.push(std::make_shared<T >(data));
        }

        size_t size(){
            std::lock_guard<std::mutex> guard(mutex);
            return stack.size();
        }   

        std::shared_ptr<T > pop()
        {
            std::shared_ptr<T > res;
            std::lock_guard<std::mutex> guard(mutex);
            if(stack.size()){
                res.swap(stack.top());
                stack.pop();
            }
            return res;
        }
};

}

#endif