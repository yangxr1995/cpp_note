#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <mutex>
using namespace std;

template <typename T>
class Queue {
    public:
        Queue<T> () 
        {
            _head = new QueueItem;
        }
        ~Queue<T> () {
            while (!empty()) {
                pop();
            }
            delete _head;
        }

        T pop() {
            return __pop(_head->_next);
        }
        T pop_back() {
            return __pop(_head->_prev);
        }

        void push(const T &a) {
            QueueItem *item = new QueueItem(a);
            item->insert(_head, _head->_next);
        }
        void push_back(const T &a) {
            QueueItem *item = new QueueItem(a);
            item->insert(_head->_prev, _head);
        }

        bool empty() const {
            return _head->_next == _head;
        }

        Queue(const Queue &) = delete;
        Queue &operator=(const Queue &) = delete;

    private:
struct QueueItem;
        T __pop(QueueItem *item) {
            if (empty())
                throw std::runtime_error("queue is empty");
            item->remove();
            T result = std::move(item->_data);
            delete item;
            return result;
        }

        struct QueueItem {
            QueueItem()
            {
                _next = this;
                _prev = this;
            }
            QueueItem(const T data)
            : _data(data) 
            {
                _next = this;
                _prev = this;
            }
            ~QueueItem() {
            }

            void insert(QueueItem *front, QueueItem *rear)  {
                _next = rear;
                _prev = front;
                front->_next = this;
                rear->_prev = this;
            }

            QueueItem *remove() {
                _prev->_next = _next;
                _next->_prev = _prev;
                return this;
            }

            static void queue_item_pool_alloc() {
                QueueItem *p;
                queue_item_pool = (QueueItem *)malloc(QUEUE_ITEM_POOL_ALLOC_SIZE * sizeof(*queue_item_pool));
                if (queue_item_pool == nullptr)
                    throw std::bad_alloc();
                for (p = queue_item_pool; p != queue_item_pool + QUEUE_ITEM_POOL_ALLOC_SIZE - 1; ++p)
                    p->_next = p + 1;
                p->_next = nullptr;
            }

            // 重载 new QueueItem
            // 先调用 operator new
            // 再调用 QueueItem()
            static void * operator new(size_t size) {
                std::lock_guard<std::mutex> lock(queue_item_pool_mutex);
                if (queue_item_pool == nullptr)
                    queue_item_pool_alloc(); 
                QueueItem *p;
                p = queue_item_pool;
                queue_item_pool = queue_item_pool->_next;
                return p;
            }

            // 重载 delete QueueItem
            // 先调用 ~QueueItem()
            // 再调用 operator delete
            static void operator delete(void *p) {
                std::lock_guard<std::mutex> lock(queue_item_pool_mutex);
                QueueItem *item = (QueueItem *)p;
                item->_next = queue_item_pool;
                queue_item_pool = item;
            }

            T _data;
            QueueItem *_next;
            QueueItem *_prev;

            static std::mutex queue_item_pool_mutex;
            static const int QUEUE_ITEM_POOL_ALLOC_SIZE;
            static QueueItem *queue_item_pool;
        };

        QueueItem *_head;

};

template <typename T>
typename Queue<T>::QueueItem * Queue<T>::QueueItem::queue_item_pool = nullptr;

template <typename T>
const int Queue<T>::QueueItem::QUEUE_ITEM_POOL_ALLOC_SIZE  = 1000;

template <typename T>
std::mutex Queue<T>::QueueItem::queue_item_pool_mutex;

class Stu {
    public:
        Stu(const string &name = "", int age = 0)
        : _name(name), _age(age) {}
        Stu(const Stu &s) 
        : _name(s._name), _age(s._age) {}
        ~Stu() = default;
    private:
        friend ostream &operator<<(ostream &out, const Stu &s);
        string _name;
        int _age;
};

ostream &operator<<(ostream &out, const Stu &s)
{
    out << "name:" << s._name << " age:" << s._age;
    return out;
}

int main (int argc, char *argv[]) {
    Queue<Stu> q;
    q.push(Stu("aaa", 11));
    q.push(Stu("bbb", 22));
    q.push(Stu("ccc", 33));

    while (!q.empty()) {
        cout << q.pop() << endl;
    }

    q.push_back(Stu("aaa", 11));
    q.push_back(Stu("bbb", 22));
    q.push_back(Stu("ccc", 33));

    while (!q.empty()) {
        cout << q.pop_back() << endl;
    }

    return 0;
}
