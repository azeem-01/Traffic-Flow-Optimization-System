#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <list>
#include <stack>
using namespace std;

template <typename T>
struct GNode
{
	T v;
	list<pair<T, float>> l;   
	GNode(T V = T())
	{
		v = V;
	}
};

const int s = 1000;
template <typename T>
class AdjList
{
	GNode<T> array[s];
	bool dir;
	int count;

public:
	AdjList(bool d = false)
	{
		dir = d;
		count = 0;
	}
	bool isEmpty()
	{
		return count == 0;
	}
	int VertixCount()
	{
		return count;
	}
	int EdgeCount()
	{
		int edges = 0;
		for (int i = 0; i < count; i++)
			edges += array[i].l.size();
		if (dir)
			return edges;
		else
			return edges / 2;
	}
	bool edgeExists(T v1, T v2)
	{
		int i = findVertix(v1);
		if (i != -1)
		{
			for (pair<T, float> p : array[i].l)
			{
				if (p.first == v2)
					return true;
			}
		}
		return false;
	}
	void insertVertix(T v)
	{
		if (count < s)
		{
			array[count].v = v;
			count++;
		}
		else
			cout << "Graph is full" << endl;
	}
	void insertEdge(T v1, T v2, float w = 1.0f)
	{
		int i = findVertix(v1);
		int j = findVertix(v2);
		if (i != -1 && j != -1)
		{
			array[i].l.push_back({ v2, w });
			if (!dir)
				array[j].l.push_back({ v1, w });
		}
		else
			cout << "One or both vertixes not found." << endl;
	}
	void updateEdgeWeight(T v1, T v2, float w) 
	{
		int i = findVertix(v1);
		if (i != -1)
		{
			for (pair<T, float>& p : array[i].l)
			{
				if (p.first == v2)
				{
					p.second = w;
					return;
				}
			}
		}
		cout << "Edge not found." << endl;
	}
	void deleteVert(T v1)
	{
		int i = findVertix(v1);
		if (i != -1)
		{
			for (int k = 0; k < count; k++)
			{
				auto it = array[i].l.begin();
				while (it != array[i].l.end())
				{
					if (it->first == v1)
						it = array[k].l.erase(it);
				}
			}
			for (int k = i; k < count - 1; k++)
			{
				array[k] = array[k + 1];
			}
			count--;
		}
		else
			cout << "Vertix not found." << endl;
	}
	void deleteEdge(T v1, T v2)
	{
		int i = findVertix(v1);
		int j = findVertix(v2);
		if (i != -1 && j != -1)
		{
			auto it = array[i].l.begin();
			while (it != array[i].l.end())
			{
				if (it->first == v2)
					it = array[i].l.erase(it);
				else
					++it;
			}
			if (!dir)
			{
				auto it2 = array[j].l.begin();
				while (it2 != array[j].l.end())
				{
					if (it2->first == v1)
						it2 = array[j].l.erase(it2);
					else
						++it2;
				}
			}
		}
		else
			cout << "One or both vertixes not found." << endl;
	}
	void search(T v)
	{
		int i = findVertix(v);
		if (i != -1)
		{
			cout << "Vertix " << v << " found. Adjacent vertixes: ";
			for (pair<T, float> p : array[i].l)
				cout << p.first << "(Weight = " << p.second << ") ";
			cout << endl;
		}
		else
			cout << "Vertix not found." << endl;
	}
	void BFS(T start)
	{
		int i = findVertix(start);
		if (i == -1)
		{
			cout << "Start vertix not found." << endl;
			return;
		}
		bool visited[s] = { 0 };
		list<T> queue;
		queue.push_back(start);
		visited[i] = true;
		while (!queue.empty())
		{
			T current = queue.front();
			queue.pop_front();
			cout << current << " ";
			int j = findVertix(current);
			for (pair<T, float> p : array[j].l)
			{
				int k = findVertix(p.first);
				if (!visited[k])
				{
					queue.push_back(p.first);
					visited[k] = true;
				}
			}
		}
		cout << endl;
	}
	void DFS(T start)
	{
		int i = findVertix(start);
		if (i == -1)
		{
			cout << "Start vertix not found." << endl;
			return;
		}
		bool visited[s] = { 0 };
		stack<T> stk;
		stk.push(start);
		while (!stk.empty())
		{
			T current = stk.top();
			stk.pop();
			int j = findVertix(current);
			if (!visited[j])
			{
				cout << current << " ";
				visited[j] = true;
				for (pair<T, float> p : array[j].l)
				{
					int k = findVertix(p.first);
					if (!visited[k])
						stk.push(p.first);
				}
			}
		}
		cout << endl;
	}
	list<T> shortestPath(T v1, T v2) //dijkstra's algorithm
	{
		int i = findVertix(v1);
		int j = findVertix(v2);
		list<T> path;
		if (i == -1 || j == -1)
		{
			cout << "One or both vertixes not found." << endl;
			return path;
		}
		float dist[s];
		int prev[s];
		bool visited[s] = { 0 };
		for (int k = 0; k < count; k++)
		{
			dist[k] = INT_MAX;
			prev[k] = -1;
		}
		dist[i] = 0;
		for (int it = 0; it < count; it++)
		{
			int u = -1;
			for (int k = 0; k < count; k++)
			{
				if (!visited[k] && (u == -1 || dist[k] < dist[u]))
					u = k;
			}
			if (u == -1 || dist[u] == INT_MAX)
				break;
			visited[u] = true;
			for (pair<T, float> p : array[u].l)
			{
				int v = findVertix(p.first);
				if (v != -1 && !visited[v] && dist[u] + p.second < dist[v])
				{
					dist[v] = dist[u] + p.second;
					prev[v] = u;
				}
			}
		}
		if (dist[j] == INT_MAX)
		{
			cout << "No path from " << v1 << " to " << v2 << endl;
			return path;
		}
		stack<int> stk;
		for (int it = j; it != -1; it = prev[it])
			stk.push(it);
		while (!stk.empty())
		{
			path.push_back(array[stk.top()].v);
			stk.pop();
		}
		return path;
	}
	void MST()
	{
		if (dir)
		{
			cout << "MST only on undirected graphs." << endl;
			return;
		}
		if (count == 0)
			return;
		bool inTree[s] = { 0 };
		float key[s];
		int parent[s];
		for (int i = 0; i < count; i++)
		{
			key[i] = INT_MAX;
			parent[i] = -1;
		}
		key[0] = 0;
		for (int it = 0; it < count; it++)
		{
			int u = -1;
			for (int i = 0; i < count; i++)
				if (!inTree[i] && (u == -1 || key[i] < key[u]))
					u = i;
			if (u == -1)
				break;
			inTree[u] = true;
			for (pair<T, float> p : array[u].l)
			{
				int v = findVertix(p.first);
				if (v != -1 && !inTree[v] && p.second < key[v])
				{
					key[v] = p.second;
					parent[v] = u;
				}
			}
		}
		cout << "MST edges:" << endl;
		float total = 0;
		for (int i = 1; i < count; i++)
		{
			if (parent[i] != -1)
			{
				cout << array[parent[i]].v << " - " << array[i].v << " : " << key[i] << endl;
				total += key[i];
			}
		}
		cout << "Total MST weight: " << total << endl;
	}
	int findVertix(T v)
	{
		for (int i = 0; i < count; i++)
		{
			if (array[i].v == v)
				return i;
		}
		return -1;
	}
};
#endif
