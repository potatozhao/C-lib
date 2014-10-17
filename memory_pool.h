#include <pthread.h>
#include <stdlib.h>
#include <string.h>
//#include "memory_pool.cpp"
template<typename T>
class MemoryPool
{
public:
    MemoryPool();
    ~MemoryPool();

    struct Node {
        Node* prev;
        Node* next;
        T data;
        Node()
        {
            prev = NULL;
            next = NULL;
        }
    };

    Node* GetFreeNode();

    int AddNode();

    int ResetNode(Node* node);

    Node* DelNode(Node* node);

    int FreeNode(Node* node);

    int GetTotalNum();

    int GetFreeNum();

    int GetUsedNum();

    int SetSize(int num);


private:
    Node* _PopNode(Node* head);
    int _PushNode(Node* head, Node* node);

private:
    int list_primary_num;
    int list_free_num;
    int list_used_num;

    Node* list_free;

    Node* list_used;

    pthread_mutex_t m_free_lock;
    pthread_mutex_t m_used_lock;

};


template <typename T>
MemoryPool<T>::MemoryPool() {

    list_primary_num = 0;
    list_free_num = 0;
    list_used_num = 0;
    list_free = new Node;
    list_used = new Node;

    pthread_mutex_init(&m_free_lock, NULL);
    pthread_mutex_init(&m_used_lock, NULL);

    for(int i = 0; i < list_primary_num;i++)
    {
        pthread_mutex_lock(&m_free_lock);
        _PushNode(list_free, new Node);
        ++list_free_num;
        pthread_mutex_unlock(&m_free_lock);
    }
}

template <typename T>
MemoryPool<T>::~MemoryPool() {
    pthread_mutex_destroy(&m_free_lock);
    pthread_mutex_destroy(&m_used_lock);
}

template <typename T>
int MemoryPool<T>::SetSize(int num) {
    list_primary_num = num;
    return 0;
}

template <typename T>
typename MemoryPool<T>::Node*
MemoryPool<T>::GetFreeNode() {
    if (list_free_num == 0){
        AddNode();
        pthread_mutex_lock(&m_free_lock);
        Node* node = _PopNode(list_free);
        --list_free_num;
        pthread_mutex_unlock(&m_free_lock);
        ResetNode(node);
        pthread_mutex_lock(&m_used_lock);
        _PushNode(list_used, node);
        ++list_used_num;
        pthread_mutex_unlock(&m_used_lock);
        return node;
    }
    pthread_mutex_lock(&m_free_lock);
    Node* node = _PopNode(list_free);
    --list_free_num;
    pthread_mutex_unlock(&m_free_lock);
    ResetNode(node);
    pthread_mutex_lock(&m_used_lock);
    _PushNode(list_used, node);
    ++list_used_num;
    pthread_mutex_unlock(&m_used_lock);
    return node;
}


template <typename T>
int MemoryPool<T>::AddNode() {
    for(int i = 0; i < list_primary_num; i++)
    {
        pthread_mutex_lock(&m_free_lock);
        _PushNode(list_free, new Node);
        ++list_free_num;
        pthread_mutex_unlock(&m_free_lock);
    }
    return 0;
}

template <typename T>
int MemoryPool<T>::ResetNode(Node *node) {
    memset(node, 0, sizeof(Node));
    return 0;
}

template <typename T>
typename MemoryPool<T>::Node*
MemoryPool<T>::DelNode(Node* node){
    if(node->next == NULL)
    {
        pthread_mutex_lock(&m_used_lock);
        node->prev->next = NULL;
        pthread_mutex_unlock(&m_used_lock);
        return node;

    }
    pthread_mutex_lock(&m_used_lock);
    node->next->prev = node->prev;
    node->next->prev->next = node->next;
    pthread_mutex_unlock(&m_used_lock);
    return node;
}

template <typename T>
int MemoryPool<T>::FreeNode(Node* node) {
    pthread_mutex_lock(&m_used_lock);
    node = DelNode(node);
    --list_used_num;
    pthread_mutex_unlock(&m_used_lock);
    ResetNode(node);
    pthread_mutex_lock(&m_free_lock);
    _PushNode(list_free, node);
    ++list_free_num;
    pthread_mutex_unlock(&m_free_lock);
    return 0;
}

template <typename T>
int MemoryPool<T>::_PushNode(Node *head, Node *node) {
    if(head->next == NULL)
    {
        head->next = node;
        node->prev = head;
        node->next = NULL;

    }
    head->next->prev = node;
    head->next->prev->prev = head;
    node->next = head->next;
    head->next = node;
    return 0;
}

template <typename T>
typename MemoryPool<T>::Node*
MemoryPool<T>::_PopNode(Node *head) {
    Node* node = head->next;
    if (head->next->next == NULL)
    {
        head->next = NULL;
        node->prev = NULL;
        node->next = NULL;
        return node;
    }
    head->next = head->next->next;
    head->next->prev = head;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

template <typename T>
int MemoryPool<T>::GetTotalNum() {
    return list_free_num+list_used_num;
}

template <typename T>
int MemoryPool<T>::GetFreeNum() {
    return list_free_num;
}

template <typename T>
int MemoryPool<T>::GetUsedNum() {
    return list_used_num;
}