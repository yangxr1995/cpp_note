// 将两个升序链表合并为一个新的 升序 链表并返回。新链表是通过拼接给定的两个链表的所有节点组成的。 


struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};


/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode() : val(0), next(nullptr) {}
 *     ListNode(int x) : val(x), next(nullptr) {}
 *     ListNode(int x, ListNode *next) : val(x), next(next) {}
 * };
 */
class Solution {
public:
    ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {
        ListNode *head = nullptr;
        ListNode **pp = &head, **pdel;

        while (list1 != nullptr && list2 != nullptr) {
            if (list1->val <= list2->val)
                pdel = &list1;
            else
                pdel = &list2;
            *pp = *pdel;
            *pdel = (*pdel)->next;
            (*pp)->next = nullptr;
            pp = &(*pp)->next;
        }
        *pp = list1 == nullptr ? list2 : list1;

        return head;
    }
};

int main (int argc, char *argv[]) {

    return 0;
}
