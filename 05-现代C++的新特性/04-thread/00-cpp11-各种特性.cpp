#include <iostream>


/*
 * C++11特性
 *
 * 1. 关键字和语法
 * auto : 根据右值推到右值类型，然后就知道左值的类型了
 * nullptr : 用于区分 NULL 和 0，以前NULL无法区分指针和整形
 * foreach : 可以遍历容器，数组等
 * for (type val : container) // 底层通过迭代器实现
 * 		cout << val << endl;
 *
 * 右值引用 : move  forward
 *
 * 模板新特性  : typename... A 可变参
 *
 * 2. 函数对象和绑定器
 * function : 将函数/lambda转换为函数对象 
 * bind : bind1st 和 bind2nd 二元函数对象 -> 一元函数对象
 * lambda
 *
 * 3. 智能指针
 * shared_ptr weak_ptr
 *
 * 4. 容器
 * 以前只有 set map 红黑树O(lgn)
 * unordered_map unordered_set 哈希表O(1)
 * array : 固定大小的数组   vector(动态大小数组)
 * forward_list : 单向链表  和 list(双向链表)
 *
 * 5. 多线程
 * thread
 * mutex
 * condition_variable
 * unique_lock
 * lock_guard
 *
 */

int main()
{
	return 0;
}
