#ifndef QUEUE_H
#define QUEUE_H
#include <iostream>
using namespace std;
template<class T>
struct QNode
{
	T d;
	QNode<T>* next;
	QNode(T data = T()) : d(data), next(NULL) {}
};
template<class T>
class Queue
{
private:
	QNode<T>* front;
	QNode<T>* rear;
public:
	Queue() :front(NULL), rear(NULL) {}
	Queue(Queue& queue)
	{
		front = rear = NULL;
		QNode<T>* temp = queue.front;
		while (temp != NULL)
		{
			enqueue(temp->d);
			temp = temp->next;
		}
	}
	T getFrontElement()
	{
		if (!isEmpty())
			return front->d;
		else
		{
			cout << "No front...queue is Empty";
			return T();
		}
	}

	T getRearElement()
	{
		if (!isEmpty())
			return rear->d;
		else
		{
			cout << "No rear...queue is Empty";
			return T();
		}
	}
	void enqueue(T d)
	{
		QNode<T>* n = new QNode<T>(d);
		if (isEmpty())
			front = rear = n;
		else
		{
			rear->next = n;
			rear = n;
		}
	}
	void dequeue()
	{
		if (isEmpty())
			cout << "Queue is already empty" << endl;
		else if (front->next == NULL)
		{
			delete front;
			front = rear = NULL;
		}
		else
		{
			QNode<T>* temp = front;
			front = front->next;
			delete temp;
			temp = NULL;
		}
	}
	bool isEmpty()
	{
		return front == NULL;
	}
	int getLength()
	{
		QNode<T>* temp = front;
		int count = 0;
		while (temp != NULL)
		{
			count++;
			temp = temp->next;
		}
		return count;
	}
	QNode<T>* getFront()
	{
		if (!isEmpty())
			return front;
		else
		{
			cout << "Queue is Empty";
			return NULL;
		}
	}
	QNode<T>* getRear()
	{
		if (!isEmpty())
			return rear;
		else
		{
			cout << "Queue is Empty";
			return NULL;
		}
	}
	~Queue()
	{
		while (!isEmpty())
			dequeue();
	}
};
#endif