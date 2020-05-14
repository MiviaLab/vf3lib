#ifndef LOCKFREESTACK_HPP
#define LOCKFREESTACK_HPP

#include <atomic>
#include "Stack.hpp"

namespace vflib
{

template<typename T >
class LockFreeStack : public Stack<T >
{
    private:
        struct Node
        {
            std::shared_ptr<T > data;
            Node* next;
            Node(T const& data_):data(std::make_shared<T >(data_)){}
        };

        struct HeadNode
        {
            uintptr_t aba = 0;
            Node* node = nullptr;
        };

        std::atomic<HeadNode> head;
        std::atomic<HeadNode> free;
        std::atomic<size_t> count;

        void push_node(std::atomic<HeadNode>& head, Node* node)
        {
            HeadNode next, origin = head.load();
            do
            {
                node->next = origin.node;
                next.aba = origin.aba + 1;
                next.node = node;
                /* Mi aspetto che head sia uguale ad origin quindi lo scambio con next */
            } while (!head.compare_exchange_weak(origin, next));
        }

        Node* pop_node(std::atomic<HeadNode>& head)
        {
            HeadNode next, origin = head.load();
            do
            {
                if(origin.node == nullptr)
                {
                    return nullptr;
                }
                next.aba = origin.aba + 1;
                next.node = origin.node->next;
            } while (!head.compare_exchange_weak(origin, next));
            return origin.node;
        }
    
    public:
        LockFreeStack(){
            count=0;
        }

        ~LockFreeStack()
        {
            Node* node = nullptr;
            HeadNode headnode = free.load();
            while(headnode.node)
            {
                node = headnode.node;
                headnode.node = headnode.node->next;
                delete node;
            }
        }

        size_t size()
        {
            return count.load();
        }

        void push(T const& data)
        {
            /* Reuse node*/
            Node* n = pop_node(free);
            if(!n)
            {
                n = new Node(data);
            }
            else
            {
                n->data = std::make_shared<T >(data);
            }
            push_node(head, n);
            count++;
        }

        std::shared_ptr<T > pop()
        {
            std::shared_ptr<T > res;
            Node* n = pop_node(head);
            if(!n)
            {
                return res;
            }
            res.swap(n->data);
            count--;
            push_node(free, n);
            return res;
        }

};

}

#endif