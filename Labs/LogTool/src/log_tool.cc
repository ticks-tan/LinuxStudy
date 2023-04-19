/**
* @File log_tool.cc
* @Date 2023-04-19
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/

#include "log_tool.h"

void LogLink::pushLogMsg(const std::string &msg)
{
    auto* node = new LogLinkNode{};
    node->msg = msg;

    node->next = this->_head;
    while (this->_lock.load() || !this->_head.compare_exchange_strong(node->next, node)) {}
    node->next->prv = node;
    this->_size.fetch_add(1);
    if (this->_size.load() == 1) {
        if (this->_tail.load() == nullptr) {
            this->_tail.store(this->_head.load());
        }
    }
}

void LogLink::clearNode()
{
    this->_lock.store(true);
    auto* node = this->_tail.load();
    LogLinkNode* prv = node;
    while (prv != nullptr) {
        prv->next = nullptr;
        prv = prv->prv;
        delete node;
        node = prv;
    }
    this->_head.store(nullptr);
    this->_tail.store(nullptr);
    this->_lock.store(false);
}

LogLinkNode *LogLink::popLogMsg()
{
    LogLinkNode* node = this->_tail.load();
    if (node == nullptr) {
        return nullptr;
    }
    while (this->_lock.load() || !this->_tail.compare_exchange_strong(node, node->prv)) {}
    node->prv->next = nullptr;
    return node;
}

bool LogLink::exchange(LogLink &link1, LogLink &link2)
{
    bool lock = link1._lock.load();
    while (!link1._lock.compare_exchange_strong(lock, true)) {}
    lock = link2._lock.load();
    while (!link2._lock.compare_exchange_strong(lock, true)) {}

    auto* node = link1._head.load();
    link1._head.store(link2._head);
    link2._head.store(node);

    node = link1._tail.load();
    link1._tail.store(link2._tail);
    link2._tail.store(node);

    size_t tmp = link1._size;
    link1._size.store(link2._size);
    link2._size.store(tmp);

    link2._lock.store(false);
    link1._lock.store(false);
    return false;
}

