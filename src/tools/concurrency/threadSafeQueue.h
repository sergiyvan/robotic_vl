/*
 * threadSafeQueue.h
 *
 *  Created on: Jul 9, 2013
 *      Author: stepuh
 */

#ifndef THREADSAFEQUEUE_H_
#define THREADSAFEQUEUE_H_

#include "ModuleFramework/Module.h"
#include <queue>


template <class T>
class ThreadSafeQueue {
public:

	unsigned int maxSize;
	std::queue<T> queue;
	CriticalSection cs;


	ThreadSafeQueue(unsigned int _maxSize) {
		maxSize = _maxSize;
		cs.setName("ThreadSafeQueue");
	}


	bool enqueue(T newElem) {
		CriticalSectionLock lock(cs);
		if (queue.size() >= maxSize) {
			return false;
		}

		queue.push(newElem);
		return true;
	}


	/**
	 * Enqueues new element in Queue.
	 * If there is not enough space left, the very first element gets deleted.
	 * @param newElem
	 * @return true, if no element had to be deleted, otherwise false.
	 */
	bool enqueue_force(T newElem) {
		CriticalSectionLock lock(cs);
		if (queue.size() >= maxSize) {
			queue.pop();
			queue.push(newElem);
			return false;
		} else {
			queue.push(newElem);
			return true;
		}
	}


	bool dequeue(T& target) {
		CriticalSectionLock lock(cs);
		if (queue.size() <= 0) {
			return false;
		}

		target = queue.front();
		queue.pop();
		return true;
	}

};

#endif /* THREADSAFEQUEUE_H_ */
